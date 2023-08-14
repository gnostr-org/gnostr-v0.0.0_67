## Nostr client and relay

Nostr client and relay is a [https://nostr.com/](https://nostr.com/)  C++ engine that allows to build Nostr applications for command line, desktop or web.

[https://github.com/pedro-vicente/nostr_client_relay](https://github.com/pedro-vicente/nostr_client_relay)

 :warning: **ATTENTION DEVELOPERS** :warning:

See [BUILDING.md](./BUILDING.md) for source code build instructions. There are 3 non exclusive modes of build (command line, desktop and web). Default is command line only.

Nostr_client_relay allows to build Nostr clients and Nostr relays. It includes:

- a command line Nostr client, **Nostro**
- a command line Nostr relay, **Vostro**
- a web interface for a client, **Wostro**
- a native desktop client, **Gnostro** (for Mac, Linux, Windows)
- a native mobile client, **Mostro** (for iOS) :warning: Android coming soon :warning:

## Nostro

Nostro is a Nostr command line client. Usage is:

```
./nostro [OPTIONS]
[OPTIONS]:
  --uri <wss URI>      Wss URI to send
  --req                message is a request (REQ). EVENT parameters are ignored
REQ OPTIONS: These are for the REQ filter, per NIP-01
  --authors <string>   a list of pubkeys or prefixes
EVENT OPTIONS: These are to publish an EVENT, per NIP-01
  --content <string>   the content of the note
  --kind <number>      set kind
  --sec <hex seckey>   set the secret key for signing, otherwise one will be randomly generated
```

## Vostro

Vostro is a Nostr relay. At the momment it uses a plain text JSON database (for development purposes).
A database is a JSON array of events read and saved to filesystem in JSON format. Vostro is a command line
application. To start Vostro, open a shell and do (the output is from the Vostro log output, Vostro
is a WebSockets server):

```
./vostro
vostro:04:23:00 Listening on port: 8080
```


## Examples 

This example shows 2 Nostro calls: publishing an EVENT and doing a REQ on the relay database. 
If no <code>--uri</code> parameter is set, then Nostro publishes to a Nostr relay listening in <code>localhost</code>.
To publish an event signed with your private key, with the content 'hello world', we use 

```
./nostro --sec <seckey> --content 'hello world' --kind 1

```

This call generated the following entry on the Vostro JSON database. To note that the key 'pubkey'
is the Nostr public key associated with the private key used by Nostro to sign the event. 


```json
[
  {
    "content": "hello world",
    "created_at": 1688794190,
    "id": "c3a4a0a20712db7249aa4d07598d5f88e31d77f95a4cc8e7bb41bd64348011f8",
    "kind": 1,
    "pubkey": "4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b",
    "sig": "4337b5361f11decd00b96bc37a87324f707f370687a7cab066c4e001145d5ee3312a3f1c1cfa6f3638e3461c8dec77468fcf35b465a16c6358ef313fca748730",
    "tags": []
  }
]
```

To query for the event we just inserted in the datbase, we do


```
./nostro  --req --authors 4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b
```

where the <code>--authors</code> paramenter is the public key corresponding to the private key used to sign the event.
This call generates the following message to send 

```json
[
 "REQ",
 "5FBD876B-EA8C-454C-9E6F-590A8A5B6DC0",
 {
  "authors": [
   "4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b"
  ],
  "kinds": [
   1
  ],
  "limit": 0
 }
]
```

Vostro responds withe following EVENT, that was found in the database by comparing the <code>--authors</code> field.

```json
[
 "EVENT",
 "031007A3-FC06-452F-8727-8160F4B9A17A",
 {
  "content": "hello world",
  "created_at": 1688794190,
  "id": "c3a4a0a20712db7249aa4d07598d5f88e31d77f95a4cc8e7bb41bd64348011f8",
  "kind": 1,
  "pubkey": "4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b",
  "sig": "4337b5361f11decd00b96bc37a87324f707f370687a7cab066c4e001145d5ee3312a3f1c1cfa6f3638e3461c8dec77468fcf35b465a16c6358ef313fca748730",
  "tags": []
 }
]
```

### Running on web

At command line

```
./wostro --http-address=0.0.0.0 --http-port=8080 --docroot=.
```

Open a browser at localhost port 8080

```
http://127.0.0.1/8080
```
## Web interface

The nostro web interface at this time allows input of a limited set of the command line options.

![Nostro on the web](https://pedro-vicente.net/images/nostro.png)

It is available at

[https://nostro.cloud/]

## API

Nostr_client_relay allows an easy integration between C++ objects like strings and vectors and Nostr JSON entities like events and filters, 
defined in [NIP-01](https://github.com/nostr-protocol/nips/blob/master/01.md), using the [JSON for modern C++ library](https://github.com/nlohmann/json)

```cpp
std::string make_request(const std::string& subscription_id, const filter_t& filter);
```

```cpp
Type get_message_type(const std::string& json);
```

```cpp
int parse_event(const std::string& json, std::string& event_id, nostr::event_t& ev);
```

```cpp
int parse_request(const std::string& json, std::string& request_id, nostr::filter_t& filter);
```

```cpp
int relay_to(const std::string& uri, const std::string& json)
```

## Examples

### Make a request

```cpp
const std::string pubkey("4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b");
std::string subscription_id = "my_id";
nostr::filter_t filter;
filter.authors.push_back(pubkey);
filter.kinds.push_back(1);
filter.limit = 1;
std::string json = nostr::make_request(subscription_id, filter);
```

The following JSON is generated, where the pubkey was inserted as an item in the filter's authors array. 

```json
[
 "REQ",
 "my_id",
 {
  "authors": [
   "4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b"
  ],
  "kinds": [
   1
  ],
  "limit": 1
 }
]
```

### Make event

```cpp
std::optional<std::string> seckey;
nostr::event_t ev;
ev.content = "hello world";
ev.kind = 1;
std::string json = nostr::make_event(ev, seckey);
```

The following JSON is generated

```json
[
 "EVENT",
 {
  "content": "hello world",
  "created_at": 1688543634,
  "id": "d4675a05eb2720b44bee08bd7c1131786f2d17ef7c1f35ee69005d5ca3377242",
  "kind": 1,
  "pubkey": "e7328fe0f6b936457b0d3fdc0e1a264e8ac80e0416f239009345750609fdc0d8",
  "sig": "472acc460529a2cf56a4ff45f6726d5aa84ff556635fc56855911ee20f055689c508f05d3c64067e919d4335076e9014f47614cd2e7d5b66ba31d8c19973b21c",
  "tags": []
 }
]
```

### Parse a request

```cpp
std::string json = R"([
 "REQ",
 "34E8C71B-C0FB-4D6D-9CBB-694A091D6A2D",
 {
  "authors": [
   "4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b"
  ],
  "kinds": [
   1
  ],
  "limit": 1
 }
])";
std::string request_id;
nostr::filter_t filter;
nostr::parse_request(json, request_id, filter);
std::cout << request_id << std::endl;
std::cout << filter.authors.at(0) << std::endl;
```

### Relay to

```cpp
std::string json = R"([
 "REQ",
 "34E8C71B-C0FB-4D6D-9CBB-694A091D6A2D",
 {
  "authors": [
   "4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b"
  ],
  "kinds": [
   1
  ],
  "limit": 1
 }
])";
std::string uri = "nos.lol";
std::vector&ltstd::string&gt& response;
nostr::relay_to(uri, json, response);
comm::to_file("response.txt", response);
for (int idx = 0; idx < response.size(); idx++)
{
  std::string message = response.at(idx);
  std::cout << message << std::endl;
}
```

The above call returned these 2 messages 

```json
["EVENT","34E8C71B-C0FB-4D6D-9CBB-694A091D6A2D",{"content":"API version 1 released. Nostr_client_relay is a Nostr C++ engine that allows to build Nostr applications for command line, desktop or web. https://pedro-vicente.net/nostro.html","created_at":1688194430,"id":"9d05a7d271e63dd47dcda1f7c7058f1ce4c903fd24dfe6fdfd72034a040a9923","kind":1,"pubkey":"4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b","sig":"c0aeca949da8b444f80009c81120b2d0059516c0c324b0d6cf523dfd1a20f78bdceec0324d0d3f9940d74b146d7e2d45b105dddd38658c4b07ec2b37ec89ff12","tags":[]}]
["EOSE","34E8C71B-C0FB-4D6D-9CBB-694A091D6A2D"]
```

### Get list of follows

To get a list of follows we start with a public key that we want to get the follows from and a relay address where the list is stored

```cpp
const std::string pubkey("4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b");
const std::string uri("nos.lol");
```

To define the request (REQ) we define a filter that has as "authors" our public key and has kind "3", defined as list of contacts in 
[NIP-02](https://github.com/nostr-protocol/nips/blob/master/02.md). A random and unique subscription ID for the request must also
ge generated, with:

```cpp
std::string subscription_id = uuid::generate_uuid_v4();
nostr::filter_t filter;
filter.authors.push_back(pubkey);
filter.kinds.push_back(3);
```

This generates the following JSON, optinally formatted for display with indentation 

```json
[
 "REQ",
 "9B874BC0-8372-4A41-9F5B-3DD6859F37F0",
 {
  "authors": [
   "4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b"
  ],
  "kinds": [
   3
  ],
  "limit": 1
 }
]
```

Finally the JSON string can be generated from the C++ objects and sent to the wire simply with

```cpp
std::string json = nostr::make_request(subscription_id, filter);
nostr::relay_to(uri, json);
```

And for that request a response is obtained according to <a href="https://github.com/nostr-protocol/nips/blob/master/02.md"> NIP-02 </a>


```json
[
 "EVENT",
 "5B695BFF-2A99-48AD-AAAE-4DF13F2DB7E4",
 {
  "content": "{\"wss://nos.lol/\":{\"write\":true,\"read\":true}}",
  "created_at": 1687486321,
  "id": "b8826c41b78bf8e4545706914e0d921b77a86192393ffb3df47f5623e6fa5b8f",
  "kind": 3,
  "pubkey": "4ea843d54a8fdab39aa45f61f19f3ff79cc19385370f6a272dda81fade0a052b",
  "sig": "b1d004f1c0f8e72c9ac0c4492086c4dfc60478156feb5d6e7075923bb6e0d738d69ed61d16d65503d6df1f09b363dccd8013bd56a5354ebbbb029f558363997e",
  "tags": [
   [
    "p",
    "c708943ea349519dcf56b2a5c138fd9ed064ad65ddecae6394eabd87a62f1770"
   ],
   [
    "p",
    "fa984bd7dbb282f07e16e7ae87b26a2a7b9b90b7246a44771f0cf5ae58018f52"
   ]
  ]
 }
]
```

---

This file is part of 'Nostr_client_relay'  
Copyright (c) 2023, Space Research Software LLC, Pedro Vicente. All rights reserved.  
See file LICENSE for full license details.  





nostril(1)

# NAME

nostril - generate nostr events

# SYNPOSIS

*nostril* [OPTIONS...]

# DESCRIPTION

*nostril* is a tool that creates and signs nostr events.

# OPTIONS

*--content*
	The text contents of the note

*--dm* <hex pubkey>
	Create a direct message. This will create a kind-4 note with the
	contents encrypted>

*--envelope*
	Wrap the event with `["EVENT", ... ]` for easy relaying

*--kind* <number>
	Set the kind of the note

*--created-at* <unix timestamp>
	Set the created at. Optional, this is set automatically.

*--mine-pubkey*
	Mine a pubkey. This may or may not be cryptographically dubious.

*--pow* <difficulty>
	Number of leading 0 bits of the id the mine for proof-of-work.

*--tag* <key> <value>
	Add a tag with a single value

*-t*
	Shorthand for --tag t <hashtag>

*-p*
	Shorthand for --tag p <hex pubkey>

*-e*
	Shorthand for --tag e <note id>


# Examples

*Generate an event*

```
$ ./nostril --sec <key> --content "this is a message"
{
	"id": "da9c36bb8206e748cf136af2a43613a5ee113cb5906a09a8d3df5386039d53ab",
	"pubkey": "4f6fa8547cf2888415522918175ea0bc0eb473287c5bd7cc459ca440bdf87d97",
	"created_at": 1660750302,
	"kind": 1,
	"tags": [],
	"content": "this is a message",
	"sig": "3e4d7d93522e54f201a22944d4d37eb4505ef1cf91c278a3f7d312b772a6c6509d1e11f146d5a003265ae10411a20057bade2365501872d2f2f24219730eed87"
}
```

*Wrap event to send to a relay*

```
$ ./nostril --envelope --sec <key> --content "hello"
[ "EVENT",
{
	"id": "ed378d3fdda785c091e9311c6e6eeb075db349a163c5e38de95946f6013a8001",
	"pubkey": "fd3fdb0d0d8d6f9a7667b53211de8ae3c5246b79bdaf64ebac849d5148b5615f",
	"created_at": 1649948103,
	"kind": 1,
	"tags": [],
	"content": "hello",
	"sig": "9d9a49bbc66d4782030b24c71416965e790214d02a54ab132d960c2b02def0371c3d93e5a60a285c55e99721599d1332450731e2c6bb1114b96b591c6967f872"
} ]
```

*Send to a relay*

```
nostril --envelope --sec <key> --content "this is a message" | websocat wss://relay.damus.io
```

*Send a nip04 DM*

```
nostril --envelope --dm <pubkey> --sec <key> --content "this is a secret" | websocat wss://relay.damus.io
```

*Mine a pubkey*

```
nostril --mine-pubkey --pow <difficulty>
```

*Reply to an event. nip10 compliant, includes the `thread_id`*

```
./nostril --envelope --sec <key> --content "this is reply message" --tag e <thread_id> --tag e <note_id> | websocat wss://relay.damus.io
```

