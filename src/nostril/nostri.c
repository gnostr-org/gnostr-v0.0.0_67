#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>

#ifdef _MSC_VER
#include "clock_gettime.h"
#define CLOCK_MONOTONIC 0
#endif

#include "cursor.h"
#include "hex.h"
#include "base64.h"
#include "aes.h"
#include "sha256.h"
#include "random.h"
#include "proof.h"
#include "nostri.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// usage
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  printf("usage: nostro [OPTIONS]\n");
  printf("\n");
  printf("  OPTIONS\n");
  printf("\n");
  printf("      --uri <string>                  URI to send (e.g 'relay.damus.io', default 'localhost:8080/nostr' for vostro listening)\n");
  printf("      --rand                          send a RAND request \n");
  printf("      Request parameters\n");
  printf("      --req                           message is a request (REQ). EVENT parameters are ignored\n");
  printf("      --id <id>                       event id (hex) to look up on the request; if none a random id is sent\n");
  printf("      Event parameters (no --req)\n");
  printf("      --content <string>              the content of the note\n");
  printf("      --dm <hex pubkey>               make an encrypted dm to said pubkey. sets kind and tags.\n");
  printf("      --kind <number>                 set kind\n");
  printf("      --created-at <unix timestamp>   set a specific created-at time\n");
  printf("      --sec <hex seckey>              set the secret key for signing, otherwise one will be randomly generated\n");
  printf("      --pow <difficulty>              number of leading 0 bits of the id to mine\n");
  printf("      --mine-pubkey                   mine a pubkey instead of id\n");
  printf("      --tag <key> <value>             add a tag\n");
  printf("      -e <event_id>                   shorthand for --tag e <event_id>\n");
  printf("      -p <pubkey>                     shorthand for --tag p <pubkey>\n");
  printf("      -t <hashtag>                    shorthand for --tag t <hashtag>\n");
  printf("\n");
  exit(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// cursor_push_escaped_char
/////////////////////////////////////////////////////////////////////////////////////////////////////

inline static int cursor_push_escaped_char(struct cursor* cur, char c)
{
  switch (c)
  {
  case '"':  return cursor_push_str(cur, "\\\"");
  case '\\': return cursor_push_str(cur, "\\\\");
  case '\b': return cursor_push_str(cur, "\\b");
  case '\f': return cursor_push_str(cur, "\\f");
  case '\n': return cursor_push_str(cur, "\\n");
  case '\r': return cursor_push_str(cur, "\\r");
  case '\t': return cursor_push_str(cur, "\\t");
  }
  return cursor_push_byte(cur, c);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// cursor_push_jsonstr
/////////////////////////////////////////////////////////////////////////////////////////////////////

static int cursor_push_jsonstr(struct cursor* cur, const char* str)
{
  int i;
  int len;

  //content can be NULL (for REQ)
  if (!str)
  {
    return 1;
  }

  len = strlen(str);

  if (!cursor_push_byte(cur, '"'))
    return 0;

  for (i = 0; i < len; i++)
  {
    if (!cursor_push_escaped_char(cur, str[i]))
      return 0;
  }

  if (!cursor_push_byte(cur, '"'))
    return 0;

  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// cursor_push_tag
/////////////////////////////////////////////////////////////////////////////////////////////////////

static int cursor_push_tag(struct cursor* cur, struct nostr_tag* tag)
{
  int i;

  if (!cursor_push_byte(cur, '['))
    return 0;

  for (i = 0; i < tag->num_elems; i++)
  {
    if (!cursor_push_jsonstr(cur, tag->strs[i]))
      return 0;
    if (i != tag->num_elems - 1)
    {
      if (!cursor_push_byte(cur, ','))
        return 0;
    }
  }

  return cursor_push_byte(cur, ']');
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// cursor_push_tags
/////////////////////////////////////////////////////////////////////////////////////////////////////

static int cursor_push_tags(struct cursor* cur, struct nostr_event* ev)
{
  int i;

  if (ev->explicit_tags)
  {
    return cursor_push_str(cur, ev->explicit_tags);
  }

  if (!cursor_push_byte(cur, '['))
    return 0;

  for (i = 0; i < ev->num_tags; i++)
  {
    if (!cursor_push_tag(cur, &ev->tags[i]))
      return 0;
    if (i != ev->num_tags - 1)
    {
      if (!cursor_push_str(cur, ","))
        return 0;
    }
  }

  return cursor_push_byte(cur, ']');
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// event_commitment
/////////////////////////////////////////////////////////////////////////////////////////////////////

int event_commitment(struct nostr_event* ev, unsigned char* buf, int buflen)
{
  char timebuf[16] = { 0 };
  char kindbuf[16] = { 0 };
  char pubkey[65];
  struct cursor cur;
  int ok;

  ok = hex_encode(ev->pubkey, sizeof(ev->pubkey), pubkey, sizeof(pubkey));
  assert(ok);

  make_cursor(buf, buf + buflen, &cur);

  snprintf(timebuf, sizeof(timebuf), "%" PRIu64 "", ev->created_at);
  snprintf(kindbuf, sizeof(kindbuf), "%d", ev->kind);

  ok =
    cursor_push_str(&cur, "[0,\"") &&
    cursor_push_str(&cur, pubkey) &&
    cursor_push_str(&cur, "\",") &&
    cursor_push_str(&cur, timebuf) &&
    cursor_push_str(&cur, ",") &&
    cursor_push_str(&cur, kindbuf) &&
    cursor_push_str(&cur, ",") &&
    cursor_push_tags(&cur, ev) &&
    cursor_push_str(&cur, ",") &&
    cursor_push_jsonstr(&cur, ev->content) &&
    cursor_push_str(&cur, "]");

  if (!ok)
    return 0;

  return cur.p - cur.start;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// make_sig
/////////////////////////////////////////////////////////////////////////////////////////////////////

static int make_sig(secp256k1_context* ctx, struct key* key, unsigned char* id, unsigned char sig[64])
{
  unsigned char aux[32];

  if (!fill_random(aux, sizeof(aux)))
  {
    return 0;
  }

  return secp256k1_schnorrsig_sign32(ctx, sig, id, &key->pair, aux);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// create_key
/////////////////////////////////////////////////////////////////////////////////////////////////////

static int create_key(secp256k1_context* ctx, struct key* key)
{
  secp256k1_xonly_pubkey pubkey;

  // Try to create a keypair with a valid context, it should only fail if the secret key is zero or out of range. 
  if (!secp256k1_keypair_create(ctx, &key->pair, key->secret))
    return 0;

  if (!secp256k1_keypair_xonly_pub(ctx, &pubkey, NULL, &key->pair))
    return 0;

  // Serialize the public key. Should always return 1 for a valid public key. 
  return secp256k1_xonly_pubkey_serialize(ctx, key->pubkey, &pubkey);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// decode_key
/////////////////////////////////////////////////////////////////////////////////////////////////////

int decode_key(secp256k1_context* ctx, const char* secstr, struct key* key)
{
  if (!hex_decode(secstr, strlen(secstr), key->secret, 32))
  {
    fprintf(stderr, "could not hex decode secret key\n");
    return 0;
  }

  return create_key(ctx, key);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// xor_mix
/////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void xor_mix(unsigned char* dest, const unsigned char* a, const unsigned char* b, int size)
{
  int i;
  for (i = 0; i < size; i++)
    dest[i] = a[i] ^ b[i];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// generate_key
/////////////////////////////////////////////////////////////////////////////////////////////////////

int generate_key(secp256k1_context* ctx, struct key* key, int* difficulty)
{
  uint64_t attempts = 0;
  uint64_t duration;
  int bits;
  double pers;
  struct timespec t1, t2;

  // If the secret key is zero or out of range (bigger than secp256k1's
  // order), we try to sample a new key. Note that the probability of this
  // happening is negligible.
  if (!fill_random(key->secret, sizeof(key->secret)))
  {
    return 0;
  }

  if (difficulty == NULL)
  {
    return create_key(ctx, key);
  }

  clock_gettime(CLOCK_MONOTONIC, &t1);
  while (1)
  {
    if (!create_key(ctx, key))
      return 0;

    attempts++;

    if ((bits = count_leading_zero_bits(key->pubkey)) >= *difficulty)
    {
      clock_gettime(CLOCK_MONOTONIC, &t2);
      duration = ((t2.tv_sec - t1.tv_sec) * 1e9L + (t2.tv_nsec - t1.tv_nsec)) / 1e6L;
      pers = (double)attempts / (double)duration;
      fprintf(stderr, "mined pubkey with %d bits after %" PRIu64 " attempts, %" PRId64 " ms, %f attempts per ms\n", bits, attempts, duration, pers);
      return 1;
    }

    // NOTE: Get a new secret key by xor mixing the current secret
    // key with the current public key. This doesn't rely on the
    // system's crypto number generator so it should be fast. There
    // shouldn't be any secret key entropy issues since we got a
    // good source of entropy from the first fill_random call at
    // the start of the function.
    xor_mix(key->secret, key->secret, key->pubkey, 32);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// init_secp_context
/////////////////////////////////////////////////////////////////////////////////////////////////////

int init_secp_context(secp256k1_context** ctx)
{
  unsigned char randomize[32];

  *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
  if (!fill_random(randomize, sizeof(randomize)))
  {
    return 0;
  }

  // Randomizing the context is recommended to protect against side-channel
  // leakage See `secp256k1_context_randomize` in secp256k1.h for more
  // information about it. This should never fail. */
  return secp256k1_context_randomize(*ctx, randomize);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// generate_event_id
/////////////////////////////////////////////////////////////////////////////////////////////////////

int generate_event_id(struct nostr_event* ev)
{
  static unsigned char buf[102400];

  int len;

  if (!(len = event_commitment(ev, buf, sizeof(buf))))
  {
    fprintf(stderr, "event_commitment: buffer out of space\n");
    return 0;
  }

  sha256((struct sha256*)ev->id, buf, len);

  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// sign_event
/////////////////////////////////////////////////////////////////////////////////////////////////////

int sign_event(secp256k1_context* ctx, struct key* key, struct nostr_event* ev)
{
  if (!make_sig(ctx, key, ev->id, ev->sig))
    return 0;

  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// make_event_from_args
/////////////////////////////////////////////////////////////////////////////////////////////////////

void make_event_from_args(struct nostr_event* ev, struct args* args)
{
  ev->created_at = args->flags & HAS_CREATED_AT ? args->created_at : time(NULL);
  ev->content = args->content;
  ev->kind = args->flags & HAS_KIND ? args->kind : 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// parse_num
/////////////////////////////////////////////////////////////////////////////////////////////////////

int parse_num(const char* arg, uint64_t* t)
{
  *t = strtol(arg, NULL, 10);
  return errno != EINVAL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// nostr_add_tag_n
/////////////////////////////////////////////////////////////////////////////////////////////////////

static int nostr_add_tag_n(struct nostr_event* ev, const char** ts, int n_ts)
{
  int i;
  struct nostr_tag* tag;

  if (ev->num_tags + 1 > MAX_TAGS)
    return 0;

  tag = &ev->tags[ev->num_tags++];

  tag->num_elems = n_ts;
  for (i = 0; i < n_ts; i++)
  {
    tag->strs[i] = ts[i];
  }

  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// nostr_add_tag
/////////////////////////////////////////////////////////////////////////////////////////////////////

int nostr_add_tag(struct nostr_event* ev, const char* t1, const char* t2)
{
  const char* ts[] = { t1, t2 };
  return nostr_add_tag_n(ev, ts, 2);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// aes_encrypt
/////////////////////////////////////////////////////////////////////////////////////////////////////

static int aes_encrypt(unsigned char* key, unsigned char* iv, unsigned char* buf, size_t buflen)
{
  struct AES_ctx ctx;
  unsigned char padding;
  int i;
  struct cursor cur;

  padding = 16 - (buflen % 16);
  make_cursor(buf, buf + buflen + padding, &cur);
  cur.p += buflen;

  for (i = 0; i < padding; i++)
  {
    if (!cursor_push_byte(&cur, padding))
    {
      return 0;
    }
  }
  assert(cur.p == cur.end);
  assert((cur.p - cur.start) % 16 == 0);

  AES_init_ctx_iv(&ctx, key, iv);
  AES_CBC_encrypt_buffer(&ctx, cur.start, cur.p - cur.start);

  return cur.p - cur.start;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// copyx
/////////////////////////////////////////////////////////////////////////////////////////////////////

static int copyx(unsigned char* output, const unsigned char* x32, const unsigned char* y32, void* data)
{
  memcpy(output, x32, 32);
  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// ensure_nonce_tag
/////////////////////////////////////////////////////////////////////////////////////////////////////

static int ensure_nonce_tag(struct nostr_event* ev, int target, int* index)
{
  char* str_target = (char*)malloc(8);
  struct nostr_tag* tag;
  int i;

  for (i = 0; i < ev->num_tags; i++)
  {
    tag = &ev->tags[i];
    if (tag->num_elems == 2 && !strcmp(tag->strs[0], "nonce"))
    {
      *index = i;
      return 1;
    }
  }

  *index = ev->num_tags;

  snprintf(str_target, 7, "%d", target);
  const char* ts[] = { "nonce", "0", str_target };

  return nostr_add_tag_n(ev, ts, 3);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// mine_event
/////////////////////////////////////////////////////////////////////////////////////////////////////

int mine_event(struct nostr_event* ev, int difficulty)
{
  char* strnonce = (char*)malloc(33);
  struct nostr_tag* tag;
  uint64_t nonce;
  int index, res;

  if (!ensure_nonce_tag(ev, difficulty, &index))
    return 0;

  tag = &ev->tags[index];
  assert(tag->num_elems == 3);
  assert(!strcmp(tag->strs[0], "nonce"));
  tag->strs[1] = strnonce;

  for (nonce = 0;; nonce++)
  {
    snprintf(strnonce, 32, "%" PRIu64, nonce);

    if (!generate_event_id(ev))
      return 0;

    if ((res = count_leading_zero_bits(ev->id)) >= difficulty)
    {
      fprintf(stderr, "mined %d bits\n", res);
      return 1;
    }
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// make_encrypted_dm
/////////////////////////////////////////////////////////////////////////////////////////////////////

int make_encrypted_dm(secp256k1_context* ctx, struct key* key, struct nostr_event* ev, unsigned char nostr_pubkey[32], int kind)
{
  size_t inl = strlen(ev->content);
  int enclen = inl + 16;
  size_t buflen = enclen * 3 + 65 * 10;
  unsigned char* buf = (unsigned char*)malloc(buflen);
  unsigned char shared_secret[32];
  unsigned char iv[16];
  unsigned char compressed_pubkey[33];
  int content_len = strlen(ev->content);
  unsigned char* encbuf = (unsigned char*)malloc(content_len + (content_len % 16) + 1);
  struct cursor cur;
  secp256k1_pubkey pubkey;

  compressed_pubkey[0] = 2;
  memcpy(&compressed_pubkey[1], nostr_pubkey, 32);

  make_cursor(buf, buf + buflen, &cur);

  if (!secp256k1_ec_seckey_verify(ctx, key->secret))
  {
    fprintf(stderr, "make_encrypted_dm: ec_seckey_verify failed\n");
    return 0;
  }

  if (!secp256k1_ec_pubkey_parse(ctx, &pubkey, compressed_pubkey, sizeof(compressed_pubkey)))
  {
    fprintf(stderr, "make_encrypted_dm: ec_pubkey_parse failed\n");
    return 0;
  }

  if (!secp256k1_ecdh(ctx, shared_secret, &pubkey, key->secret, copyx, NULL))
  {
    fprintf(stderr, "make_encrypted_dm: secp256k1_ecdh failed\n");
    return 0;
  }

  if (!fill_random(iv, sizeof(iv)))
  {
    fprintf(stderr, "make_encrypted_dm: fill_random failed\n");
    return 0;
  }

  fprintf(stderr, "shared_secret ");
  print_hex(shared_secret, 32);

  memcpy(encbuf, ev->content, strlen(ev->content));
  enclen = aes_encrypt(shared_secret, iv, encbuf, strlen(ev->content));
  if (enclen == 0)
  {
    fprintf(stderr, "make_encrypted_dm: aes_encrypt failed\n");
    free(buf);
    return 0;
  }

  if ((enclen = base64_encode((char*)buf, buflen, (const char*)encbuf, enclen)) == -1)
  {
    fprintf(stderr, "make_encrypted_dm: base64 encode of encrypted fata failed\n");
    return 0;
  }
  cur.p += enclen;

  if (!cursor_push_str(&cur, "?iv="))
  {
    fprintf(stderr, "make_encrypted_dm: buffer too small\n");
    return 0;
  }

  if ((enclen = base64_encode((char*)cur.p, cur.end - cur.p, (const char*)iv, 16)) == -1)
  {
    fprintf(stderr, "make_encrypted_dm: base64 encode of iv failed\n");
    return 0;
  }
  cur.p += enclen;

  if (!cursor_push_byte(&cur, 0))
  {
    fprintf(stderr, "make_encrypted_dm: out of memory by 1 byte!\n");
    return 0;
  }

  ev->content = (const char*)cur.start;
  ev->kind = kind;

  if (!hex_encode(nostr_pubkey, 32, (char*)cur.p, cur.end - cur.p))
    return 0;

  if (!nostr_add_tag(ev, "p", (const char*)cur.p))
  {
    fprintf(stderr, "too many tags\n");
    return 0;
  }

  cur.p += 65;

  free(encbuf);
  return 1;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// parse_args
// new arguments are
// --uri <URI>
// --req <evend_id>
// --rand
/////////////////////////////////////////////////////////////////////////////////////////////////////

int parse_args(int argc, const char* argv[], struct args* args, struct nostr_event* ev)
{
  const char* arg, * arg2;
  uint64_t n;
  int has_added_tags = 0;
  int req_mode = 0;

  //detect REQ mode
  for (int idx = 1; idx < argc; idx++)
  {
    if (strcmp(argv[idx], "--req") == 0)
    {
      req_mode = 1;
      break;
    }
  }

  argv++; argc--;
  for (; argc; )
  {
    arg = *argv++; argc--;

    if (!strcmp(arg, "--help"))
    {
      usage();
    }

    if (!argc && req_mode == 0)
    {
      fprintf(stderr, "expected argument: '%s'\n", arg);
      return 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // new arguments
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    if (!strcmp(arg, "--uri"))
    {
      args->uri = *argv++; argc--;
    }

    else if (!strcmp(arg, "--req"))
    {
      args->req = 1;
    }

    else if (!strcmp(arg, "--rand"))
    {
      args->rand_req = 1;
    }

    else if (!strcmp(arg, "--id"))
    {
      args->event_id = *argv++; argc--;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // original arguments (Note: --envelope is always used) 
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    else if (!strcmp(arg, "--sec"))
    {
      args->sec = *argv++; argc--;
    }
    else if (!strcmp(arg, "--created-at"))
    {
      arg = *argv++; argc--;
      if (!parse_num(arg, &args->created_at))
      {
        fprintf(stderr, "created-at must be a unix timestamp\n");
        return 0;
      }
      else
      {
        args->flags |= HAS_CREATED_AT;
      }
    }
    else if (!strcmp(arg, "--kind"))
    {
      arg = *argv++; argc--;
      if (!parse_num(arg, &n))
      {
        fprintf(stderr, "kind should be a number, got '%s'\n", arg);
        return 0;
      }
      args->kind = (int)n;
      args->flags |= HAS_KIND;
    }
    else if (!strcmp(arg, "--envelope"))
    {
      args->flags |= HAS_ENVELOPE;
    }
    else if (!strcmp(arg, "--tags"))
    {
      if (args->flags & HAS_DIFFICULTY)
      {
        fprintf(stderr, "can't combine --tags and --pow (yet)\n");
        return 0;
      }
      if (has_added_tags)
      {
        fprintf(stderr, "can't combine --tags and --tag (yet)");
        return 0;
      }
      arg = *argv++; argc--;
      args->tags = arg;
    }
    else if (!strcmp(arg, "-e"))
    {
      has_added_tags = 1;
      arg = *argv++; argc--;
      if (!nostr_add_tag(ev, "e", arg))
      {
        fprintf(stderr, "couldn't add e tag");
        return 0;
      }
    }
    else if (!strcmp(arg, "-p"))
    {
      has_added_tags = 1;
      arg = *argv++; argc--;
      if (!nostr_add_tag(ev, "p", arg))
      {
        fprintf(stderr, "couldn't add p tag");
        return 0;
      }
    }
    else if (!strcmp(arg, "-t"))
    {
      has_added_tags = 1;
      arg = *argv++; argc--;
      if (!nostr_add_tag(ev, "t", arg))
      {
        fprintf(stderr, "couldn't add t tag");
        return 0;
      }
    }
    else if (!strcmp(arg, "--tag"))
    {
      has_added_tags = 1;
      if (args->tags)
      {
        fprintf(stderr, "can't combine --tag and --tags (yet)");
        return 0;
      }
      arg = *argv++; argc--;
      if (argc == 0)
      {
        fprintf(stderr, "expected two arguments to --tag\n");
        return 0;
      }
      arg2 = *argv++; argc--;
      if (!nostr_add_tag(ev, arg, arg2))
      {
        fprintf(stderr, "couldn't add tag '%s' '%s'\n", arg, arg2);
        return 0;
      }
    }
    else if (!strcmp(arg, "--mine-pubkey"))
    {
      args->flags |= HAS_MINE_PUBKEY;
    }
    else if (!strcmp(arg, "--pow"))
    {
      if (args->tags)
      {
        fprintf(stderr, "can't combine --tags and --pow (yet)\n");
        return 0;
      }
      arg = *argv++; argc--;
      if (!parse_num(arg, &n))
      {
        fprintf(stderr, "could not parse difficulty as number: '%s'\n", arg);
        return 0;
      }
      args->difficulty = n;
      args->flags |= HAS_DIFFICULTY;
    }
    else if (!strcmp(arg, "--dm"))
    {
      arg = *argv++; argc--;
      if (!hex_decode(arg, strlen(arg), args->encrypt_to, 32))
      {
        fprintf(stderr, "could not decode encrypt-to pubkey");
        return 0;
      }
      args->flags |= HAS_ENCRYPT;
    }
    else if (!strcmp(arg, "--content"))
    {
      arg = *argv++; argc--;
      args->content = arg;
    }
    else
    {
      fprintf(stderr, "unexpected argument '%s'\n", arg);
      return 0;
    }
  }

  if (!args->content)
    args->content = "";

  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// print_event
/////////////////////////////////////////////////////////////////////////////////////////////////////

int print_event(struct nostr_event* ev, char** json)
{
  unsigned char buf[102400];
  char pubkey[65];
  char id[65];
  char sig[129];
  struct cursor cur;
  int ok;

  ok = hex_encode(ev->id, sizeof(ev->id), id, sizeof(id)) &&
    hex_encode(ev->pubkey, sizeof(ev->pubkey), pubkey, sizeof(pubkey)) &&
    hex_encode(ev->sig, sizeof(ev->sig), sig, sizeof(sig));

  assert(ok);

  make_cursor(buf, buf + sizeof(buf), &cur);
  if (!cursor_push_tags(&cur, ev))
    return 0;

  char str[1024];
  char out[102400];

  sprintf(str, "[\"EVENT\",");
  strcpy(out, str);

  sprintf(str, "{\"id\": \"%s\",", id);
  strcat(out, str);
  sprintf(str, "\"pubkey\": \"%s\",", pubkey);
  strcat(out, str);
  sprintf(str, "\"created_at\": %" PRIu64 ",", ev->created_at);
  strcat(out, str);
  sprintf(str, "\"kind\": %d,", ev->kind);
  strcat(out, str);
  sprintf(str, "\"tags\": %.*s,", (int)cursor_len(&cur), cur.start);
  strcat(out, str);

  reset_cursor(&cur);
  if (!cursor_push_jsonstr(&cur, ev->content))
    return 0;

  sprintf(str, "\"content\": %.*s,", (int)cursor_len(&cur), cur.start);
  strcat(out, str);
  sprintf(str, "\"sig\": \"%s\"}", sig);
  strcat(out, str);

  sprintf(str, "]\n");
  strcat(out, str);

  fprintf(stderr, "%s", out);
  int len = strlen(out);
  strcpy(*json, out);
  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// print_request
// Clients can send 3 types of messages, which must be JSON arrays, according to the following patterns:
//
//    ["EVENT", <event JSON as defined above>], used to publish events.
//    ["REQ", <subscription_id>, <filters JSON>...], used to request events and subscribe to new updates.
//    ["CLOSE", <subscription_id>], used to stop previous subscriptions.
//
// <subscription_id> is an arbitrary, non-empty string of max length 64 chars, that should be used to represent a subscription.
//
// <filters> is a JSON object that determines what events will be sent in that subscription, it can have the following attributes:
//
// {
//  "ids": <a list of event ids or prefixes>,
//  "authors": <a list of pubkeys or prefixes, the pubkey of an event must be one of these>,
//  "kinds": <a list of a kind numbers>,
//  "#e": <a list of event ids that are referenced in an "e" tag>,
//  "#p": <a list of pubkeys that are referenced in a "p" tag>,
//  "since": <an integer unix timestamp, events must be newer than this to pass>,
//  "until": <an integer unix timestamp, events must be older than this to pass>,
//  "limit": <maximum number of events to be returned in the initial query>
// }
/////////////////////////////////////////////////////////////////////////////////////////////////////

int print_request(struct nostr_event* ev, struct args* args, char** json)
{
  char id[65];
  char str[1024];
  char out[102400];

  int limit = 5;

  //["REQ", "RAND", {"kinds": [1], "limit": 2}]
  if (args->rand_req)
  {
    sprintf(str, "[\"REQ\",");
    strcpy(out, str);
    sprintf(str, "\"RAND\",");
    strcat(out, str);
    sprintf(str, "{");
    strcat(out, str);
    sprintf(str, "\"kinds\": [1], ");
    strcat(out, str);
    sprintf(str, "\"limit\": %d", limit);
    strcat(out, str);
    sprintf(str, "}]\n");
    strcat(out, str);
  }
  else
  {
    sprintf(str, "[\"REQ\",");
    strcpy(out, str);
    sprintf(str, "\"subscription_nostro\","); //hardcoded subscription_id
    strcat(out, str);
    sprintf(str, "{");
    strcat(out, str);
    sprintf(str, "\"kinds\": [1],");
    strcat(out, str);
    if (!args->event_id)
    {
      int ok = hex_encode(ev->id, sizeof(ev->id), id, sizeof(id));
      assert(ok);
    }
    else
    {
      strcpy(id, args->event_id);
    }
    sprintf(str, "\"ids\": [\"%s\"]", id); //"ids" must be an array (only 1 element)
    strcat(out, str);
    sprintf(str, "}]\n");
    strcat(out, str);
  }

  fprintf(stderr, "%s", out);
  int len = strlen(out);
  strcpy(*json, out);
  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// make_message
/////////////////////////////////////////////////////////////////////////////////////////////////////

int make_message(struct args* args, struct nostr_event* ev, char** json)
{
  struct key key;
  secp256k1_context* ctx;
  if (!init_secp_context(&ctx))
    return 2;

  args->flags |= HAS_ENVELOPE;
  if (args->tags)
  {
    ev->explicit_tags = args->tags;
  }

  make_event_from_args(ev, args);

  if (args->sec)
  {
    if (!decode_key(ctx, args->sec, &key))
    {
      return 8;
    }
  }
  else
  {
    int* difficulty = NULL;
    if ((args->flags & HAS_DIFFICULTY) && (args->flags & HAS_MINE_PUBKEY))
    {
      difficulty = &args->difficulty;
    }

    if (!generate_key(ctx, &key, difficulty))
    {
      fprintf(stderr, "could not generate key\n");
      return 4;
    }
    fprintf(stderr, "secret_key ");
    print_hex(key.secret, sizeof(key.secret));
    fprintf(stderr, "\n");
  }

  if (args->flags & HAS_ENCRYPT)
  {
    int kind = args->flags & HAS_KIND ? args->kind : 4;
    if (!make_encrypted_dm(ctx, &key, ev, args->encrypt_to, kind))
    {
      fprintf(stderr, "error making encrypted dm\n");
      return 0;
    }
  }

  // set the event's pubkey
  memcpy(ev->pubkey, key.pubkey, 32);

  if (args->flags & HAS_DIFFICULTY && !(args->flags & HAS_MINE_PUBKEY))
  {
    if (!mine_event(ev, args->difficulty))
    {
      fprintf(stderr, "error when mining id\n");
      return 22;
    }
  }
  else
  {
    if (!generate_event_id(ev))
    {
      fprintf(stderr, "could not generate event id\n");
      return 5;
    }
  }

  if (!sign_event(ctx, &key, ev))
  {
    fprintf(stderr, "could not sign event\n");
    return 6;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // envelop is either EVENT (req 0) or REQ (req 1). If REQ return
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (args->req == 1)
  {
    if (!print_request(ev, args, json))
    {
      fprintf(stderr, "buffer too small\n");
      return 88;
    }
    return 0;
  }
  else if (!print_event(ev, json))
  {
    fprintf(stderr, "buffer too small\n");
    return 88;
  }

  return 0;
}
