## act is a docker based CI tool that reuses github actions files.
## using if: ${{ env.ACT }}
## or    if: ${{ !env.ACT }}
## can be used too control the actions of a workflow

.ONESHELL:
docker-start:## 	start docker
	touch requirements.txt
	test -d .venv || $(PYTHON3) -m virtualenv .venv
	( \
	   source .venv/bin/activate; pip install -q -r requirements.txt; \
	);
	( \
	    while ! docker system info > /dev/null 2>&1; do\
	    echo 'Waiting for docker to start...';\
	    if [[ '$(OS)' == 'Linux' ]]; then\
	     systemctl restart docker.service;\
	    fi;\
	    if [[ '$(OS)' == 'Darwin' ]]; then\
	     type -P docker && open --background -a /./Applications/Docker.app/Contents/MacOS/Docker;\
	    fi;\
	sleep 1;\
	done\
	)

## --verbose -v
## --bind -b
## --reuse -r
## are useful flags

## caution!
## binding the repo to an act instance
## can have side effects if a matrix
## is running in parallel!
## additionally
## you can use the act -C <sub_folder> -W ...
## to change the context of the workflow



act-install:## 	install act from deps/gnostr-act/install.sh -b
	@git submodule update --init --recursive  deps/gnostr-act
	./deps/gnostr-act/install.sh -b /usr/local/bin && exec bash
ubuntu-git:submodules docker-start## 	run act in .github
	#we use -b to bind the repo to the act container
	#in the single dep instances we reuse (-r) the container
	@type -P act && GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && act $(VERBOSE) $(BIND) $(REUSE) -W $(PWD)/.github/workflows/$@.yml || $(MAKE) act-install
.PHONY:deps/jq/.github/workflows/linux ubuntu-jq
#ubuntu-jq:submodules docker-start jq/.github/workflows/linux## 	run act for deps/jq/.github/workflows/linux.yml
ubuntu-jq:submodules docker-start## 	run act .github/workflows/ubuntu-jq.yml
	##@pushd deps/jq && autoreconf -i && ./configure  --disable-maintainer-mode  && make all install
	@type -P act && GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && act $(VERBOSE) $(BIND) $(REUSE) -W $(PWD)/.github/workflows/$@.yml || $(MAKE) act-install
jq/.github/workflows/linux:submodules docker-start## 	
	#we use -b to bind the repo to the act container
	#in the single dep instances we reuse (-r) the container
	@type -P act && GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && act $(VERBOSE) $(BIND) $(REUSE) -W $(PWD)/.github/workflows/$@.yml || $(MAKE) act-install
ubuntu-gnostr-cat:submodules docker-start## 	run act in .github
	#we use -b to bind the repo to the act container
	#in the single dep instances we reuse (-r) the container
	type -P act && GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && act $(VERBOSE) $(BIND) $(REUSE) -W $(PWD)/.github/workflows/$@.yml || $(MAKE) act-install
	#the matrix/pre/release builds are for the resulting app builds

## Try:
## make ubuntu-matrix reuse=true bind=true
ubuntu-matrix:submodules docker-start### 	run act/github workflow
##ubuntu-matrix
## 	make ubuntu-matrix reuse=true bind=true verbose=true
	@bash -c "export GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && echo $(GITHUB_TOKEN)"
	type -P act && GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && act -v         $(BIND) $(REUSE) -W $(PWD)/.github/workflows/$@.yml || $(MAKE) act-install

alpine-matrix:docker-start## 	run act in .github
	@bash -c "export GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && echo $(GITHUB_TOKEN)"
	type -P act && GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && act $(VERBOSE) $(BIND) $(REUSE) -W $(PWD)/.github/workflows/$@.yml || $(MAKE) act-install
##alpine-matrix
## 	make alpine-matrix reuse=true bind=true verbose=true
##	
linux-matrix:docker-start ubuntu-matrix## 	linux-matrix
	$(MAKE) alpine-matrix ubuntu-secp ubuntu-gnostr ubuntu-gnostr-relay verbose=true bind=true reuse=true
ubuntu-secp:docker-start## 	ubuntu-secp
	@bash -c "export GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && echo $(GITHUB_TOKEN)"
	type -P act && GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && act $(VERBOSE) $(BIND) $(REUSE) -W $(PWD)/.github/workflows/$@.yml || $(MAKE) act-install
ubuntu-gnostr:docker-start## 	ubuntu-gnostr
	@bash -c "export GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && echo $(GITHUB_TOKEN)"
	type -P act && GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && act $(VERBOSE) $(BIND) $(REUSE) -W $(PWD)/.github/workflows/$@.yml || $(MAKE) act-install
ubuntu-gnostr-relay:docker-start## 	ubuntu-gnostr-relay
	@bash -c "export GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && echo $(GITHUB_TOKEN)"
	type -P act && GITHUB_TOKEN=$(shell cat ~/GITHUB_TOKEN.txt) && act $(VERBOSE) $(BIND) $(REUSE) -W $(PWD)/.github/workflows/$@.yml || $(MAKE) act-install
##linux-matrix
## 	make alpine-matrix ubuntu-matrix verbose=true bind=true reuse=true
##[act.mk developement notes]
## 	use reuse=false to force a rebuild of the docker image
##[command example]
##make alpine-matrix ubuntu-matrix verbose=true bind=true reuse=true
