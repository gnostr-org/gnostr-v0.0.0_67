
ifeq ($(project),)
PROJECT_NAME                            := $(notdir $(PWD))
else
PROJECT_NAME                            := $(project)
endif
export PROJECT_NAME
VERSION                                 :=$(shell cat version)
export VERSION
TIME                                    :=$(shell date +%s)
export TIME

OS                                      :=$(shell uname -s)
export OS
OS_VERSION                              :=$(shell uname -r)
export OS_VERSION
ARCH                                    :=$(shell uname -m)
export ARCH
ifeq ($(ARCH),x86_64)
TRIPLET                                 :=x86_64-linux-gnu
export TRIPLET
endif
ifeq ($(ARCH),arm64)
TRIPLET                                 :=aarch64-linux-gnu
export TRIPLET
endif
ifeq ($(ARCH),arm64)
TRIPLET                                 :=aarch64-linux-gnu
export TRIPLET
endif

HOMEBREW                                :=$(shell which brew || false)

ifeq ($(verbose),true)
VERBOSE                                 :=-v
else
VERBOSE                                 :=
endif
export VERBOSE

ifeq ($(reuse),true)
REUSE                                   :=-r
else
REUSE                                   :=
endif
export REUSE

ifeq ($(bind),true)
BIND                                    :=-b
else
BIND                                    :=
endif
export BIND

ifeq ($(token),)
GITHUB_TOKEN                            :=$(shell cat ~/GITHUB_TOKEN.txt || echo "0")
else
GITHUB_TOKEN                            :=$(shell echo $(token))
endif
export GITHUB_TOKEN

export $(cat ~/GITHUB_TOKEN) && make act

PYTHON                                  := $(shell which python)
export PYTHON
PYTHON2                                 := $(shell which python2)
export PYTHON2
PYTHON3                                 := $(shell which python3)
export PYTHON3

PIP                                     := $(shell which pip)
export PIP
PIP2                                    := $(shell which pip2)
export PIP2
PIP3                                    := $(shell which pip3)
export PIP3

PYTHON_VENV                             := $(shell python -c "import sys; sys.stdout.write('1') if hasattr(sys, 'base_prefix') else sys.stdout.write('0')")
PYTHON3_VENV                            := $(shell python3 -c "import sys; sys.stdout.write('1') if hasattr(sys, 'real_prefix') else sys.stdout.write('0')")

python_version_full                     := $(wordlist 2,4,$(subst ., ,$(shell python3 --version 2>&1)))
python_version_major                    := $(word 1,${python_version_full})
python_version_minor                    := $(word 2,${python_version_full})
python_version_patch                    := $(word 3,${python_version_full})

my_cmd.python.3                         := $(PYTHON3) some_script.py3
my_cmd                                  := ${my_cmd.python.${python_version_major}}

PYTHON_VERSION                          := ${python_version_major}.${python_version_minor}.${python_version_patch}
PYTHON_VERSION_MAJOR                    := ${python_version_major}
PYTHON_VERSION_MINOR                    := ${python_version_minor}

export python_version_major
export python_version_minor
export python_version_patch
export PYTHON_VERSION

#GIT CONFIG
GIT_USER_NAME                           := $(shell git config user.name || echo $(PROJECT_NAME))
export GIT_USER_NAME
GH_USER_NAME                            := $(shell git config user.name || echo $(PROJECT_NAME))
#MIRRORS
GH_USER_REPO                            := $(GH_USER_NAME).github.io
GH_USER_SPECIAL_REPO                    := $(GH_USER_NAME)
KB_USER_REPO                            := $(GH_USER_NAME).keybase.pub
#GITHUB RUNNER CONFIGS
ifneq ($(ghuser),)
GH_USER_NAME := $(ghuser)
GH_USER_SPECIAL_REPO := $(ghuser)/$(ghuser)
endif
ifneq ($(kbuser),)
KB_USER_NAME := $(kbuser)
KB_USER_REPO := $(kbuser).keybase.pub
endif
export GIT_USER_NAME
export GH_USER_REPO
export GH_USER_SPECIAL_REPO
export KB_USER_REPO

GIT_USER_EMAIL                          := $(shell git config user.email || echo $(PROJECT_NAME))
export GIT_USER_EMAIL
GIT_SERVER                              := https://github.com
export GIT_SERVER
GIT_SSH_SERVER                          := git@github.com
export GIT_SSH_SERVER
GIT_PROFILE                             := $(shell git config user.name || echo $(PROJECT_NAME))
export GIT_PROFILE
GIT_BRANCH                              := $(shell git rev-parse --abbrev-ref HEAD 2>/dev/null || \
                                           echo $(PROJECT_NAME))
export GIT_BRANCH
GIT_HASH                                := $(shell git rev-parse --short HEAD 2>/dev/null || \
                                           echo $(PROJECT_NAME))
export GIT_HASH
GIT_PREVIOUS_HASH                       := $(shell git rev-parse --short master@{1} 2>/dev/null || \
                                           echo $(PROJECT_NAME))
export GIT_PREVIOUS_HASH
GIT_REPO_ORIGIN                         := $(shell git remote get-url origin 2>/dev/null || \
                                           echo $(PROJECT_NAME))
export GIT_REPO_ORIGIN
GIT_REPO_NAME                           := $(PROJECT_NAME)
export GIT_REPO_NAME
GIT_REPO_PATH                           := $(HOME)/$(GIT_REPO_NAME)
export GIT_REPO_PATH



.PHONY:- help
-:
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?##/ {printf "\033[36m%-15s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)
	@echo
more:## 	more help
	@sed -n 's/^##//p' ${MAKEFILE_LIST} | column -t -s ':' |  sed -e 's/^/	/'
	#$(MAKE) -f Makefile help

-include Makefile

##initialize
##	git submodule update --init --recursive
initialize:## 	ensure submodules exist
	git submodule update --init --recursive

.ONESHELL:
docker-start:venv
##docker-start
##	start docker on Linux or Darwin
	@touch requirements.txt && $(PYTHON3) -m pip install -q -r requirements.txt
	@test -d .venv || $(PYTHON3) -m virtualenv .venv
	@( \
	   . .venv/bin/activate; pip install -q -r requirements.txt; \
	   python3 -m pip install -q pipenv \
	   pip install -q --upgrade pip; \
	);
	@( \
	    while ! docker system info > /dev/null 2>&1; do\
	    echo 'Waiting for docker to start...';\
	    if [[ '$(OS)' == 'Linux' ]]; then\
	     type -P systemctl && systemctl restart docker.service || type -P service && service restart docker;\
	    fi;\
	    if [[ '$(OS)' == 'Darwin' ]]; then\
	     type -P docker && open --background -a /./Applications/Docker.app/Contents/MacOS/Docker;\
	    fi;\
	sleep 1;\
	done\
	)

detect:
##detect
##	detect uname -s uname -m uname -p and install sequence
##	install nvm sequence
	@bash -c "curl https://raw.githubusercontent.com/creationix/nvm/master/install.sh | bash && export NVM_DIR='$(HOME)/.nvm'; [ -s '$(NVM_DIR)/nvm.sh' ] && \. '$(NVM_DIR)/nvm.sh'; [ -s '$(NVM_DIR)/bash_completion' ] && \. '$(NVM_DIR)/bash_completion' &"
##	install rustup sequence
	$(shell echo which rustup) || curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | bash -s -- -y --no-modify-path --default-toolchain stable --profile default & source "$(HOME)/.cargo/env"
##	install if Darwin sequence
	bash -c "[ '$(shell uname -s)' == 'Darwin' ] && echo 'is Darwin' && $(shell echo which brew) && $(shell echo which cmake) && $(shell echo which rustup) && $(shell echo which cargo) && $(shell echo which gettext) && $(shell echo which autoconf) && $(shell echo which node) && $(shell echo which go) || echo"
	bash -c "[ '$(shell uname -s)' == 'Darwin' ] && type -P cmake || brew install -q --cask cmake || which cmake || echo 'not Darwin';"
	bash -c "[ '$(shell uname -s)' == 'Darwin' ] && brew install -q autoconf automake coreutils gettext golang mercurial node@14 python3 virtualenv zlib || echo 'not Darwin';"
##	install if Linux sequence
	bash -c "[ '$(shell uname -s)' == 'Linux' ] && echo 'is Linux' && $(shell echo which apt-get) &&  apt-get install autoconf bison build-essential clang cmake-curses-gui cmake expat gettext golang-go libcurl4-openssl-dev libexpat1-dev libssl-dev libtool mercurial npm pandoc pkg-config python3 python3-pip  python-is-python3 util-linux virtualenv zlib* --fix-missing && $(shell echo which cargo) || echo 'add apt-get install sequence';"
##	install gvm sequence
	@rm -rf $(HOME)/.gvm
	@bash -c "bash < <(curl -s -S -L https://raw.githubusercontent.com/moovweb/gvm/master/binscripts/gvm-installer) || echo"
	bash -c "[ '$(shell uname -m)' == 'x86_64' ] && echo 'is x86_64' || echo 'not x86_64';"
	bash -c "[ '$(shell uname -m)' == 'arm64' ] && [ '$(shell uname -s)' == 'Darwin' ] && type -P brew && brew install pandoc || echo 'not arm64 AND Darwin';"
	bash -c "[ '$(shell uname -m)' == 'i386' ] && echo 'is i386' || echo 'not i386';"

.PHONY: report
report:## 	print make variables
##	print make variables
	@echo ''
	@echo 'TIME=${TIME}'
	@echo 'PROJECT_NAME=${PROJECT_NAME}'
	@echo 'VERSION=${VERSION}'
	@echo ''
	@echo 'OS=${OS}'
	@echo 'OS_VERSION=${OS_VERSION}'
	@echo 'ARCH=${ARCH}'
	@echo ''
	@echo 'PYTHON=${PYTHON}'
	@echo 'PYTHON2=${PYTHON2}'
	@echo 'PYTHON3=${PYTHON3}'
	@echo ''
	@echo 'PIP=${PIP}'
	@echo 'PIP2=${PIP2}'
	@echo 'PIP3=${PIP3}'
	@echo ''
	@echo 'PYTHON_VENV=${PYTHON_VENV}'
	@echo 'PYTHON3_VENV=${PYTHON3_VENV}'
	@echo ''
	@echo 'GIT_USER_NAME=${GIT_USER_NAME}'
	@echo 'GH_USER_REPO=${GH_USER_REPO}'
	@echo 'GH_USER_SPECIAL_REPO=${GH_USER_SPECIAL_REPO}'
	@echo 'KB_USER_REPO=${KB_USER_REPO}'
	@echo 'GIT_USER_EMAIL=${GIT_USER_EMAIL}'
	@echo 'GIT_SERVER=${GIT_SERVER}'
	@echo 'GIT_PROFILE=${GIT_PROFILE}'
	@echo 'GIT_BRANCH=${GIT_BRANCH}'
	@echo 'GIT_HASH=${GIT_HASH}'
	@echo 'GIT_PREVIOUS_HASH=${GIT_PREVIOUS_HASH}'
	@echo 'GIT_REPO_ORIGIN=${GIT_REPO_ORIGIN}'
	@echo 'GIT_REPO_NAME=${GIT_REPO_NAME}'
	@echo 'GIT_REPO_PATH=${GIT_REPO_PATH}'

checkbrew:## 	install brew command
##	install brew command
ifeq ($(HOMEBREW),)
	@/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
else
	@type -P brew && brew install wxWidgets openssl@3.0 gettext
	$(MAKE) detect
##	brew install wxWidgets openssl@3.0 gettext
##	make detect
endif

tag:## 	git tag & git push
tags:tag
##tag 
##	git tag $(OS)-$(OS_VERSION)-$(ARCH)-$(shell date +%s)
	@git tag $(OS)-$(OS_VERSION)-$(ARCH)-$(shell date +%s)
	@git push -f --tags || echo "unable to push tags..."

-include gnostr.mk
-include venv.mk
-include act.mk
-include clean.mk

# vim: set noexpandtab:
# vim: set setfiletype make
