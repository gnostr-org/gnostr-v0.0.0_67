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

CLANG=$(shell which clang)
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
helpp:## 	
	@sed -n 's/^##//p' ${MAKEFILE_LIST} | column -t -s ':' |  sed -e 's/^/ /'

-include Makefile

##initialize
##	git submodule update --init --recursive
initialize:## 	ensure submodules exist
	git submodule update --init --recursive

detect:
##detect
##	detect uname -s uname -m uname -p
	@[[ '$(shell uname -s)' == 'Darwin' ]] && \
		echo "is Darwin" || \
		echo "not Darwin";
	@[[ '$(shell uname -s)' == 'Linux'* ]] && \
		echo "is Linux" || \
		echo "not Linux";
	@[[ '$(shell uname -m)' == 'x86_64' ]] && \
		echo "is x86_64" || \
		echo "not x86_64";
	@[[ '$(shell uname -p)' == 'i386' ]]   && \
		echo "is i386" || \
		echo "not i386";

.PHONY: report
report:## 	
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

extra:## 	additional
##extra
	@echo "example: add additional make commands"

.ONESHELL:
.PHONY:ext/boost_1_82_0/b2
ext/boost_1_82_0/b2:
	cd ext/boost_1_82_0 && ./bootstrap.sh
.PHONY:ext/boost_1_82_0/stage/lib
ext/boost_1_82_0/stage/lib:ext/boost_1_82_0/b2
	cd ext/boost_1_82_0 && ./b2 headers && ./b2
.PHONY:ext/boost_1_82_0/.git
ext/boost_1_82_0/.git:
	@devtools/refresh-submodules.sh ext/boost_1_82_0
boost:ext/boost_1_82_0/.git## 	boost

.ONESHELL:
.PHONY:ext/openssl-3.0.5
ext/openssl-3.0.5:
	cd ext/openssl-3.0.5
.PHONY:ext/openssl-3.0.5/Makefile
ext/openssl-3.0.5/Makefile:ext/openssl-3.0.5
	cd ext/openssl-3.0.5 && ./Configure && make
.PHONY:ext/openssl-3.0.5/.git
ext/openssl-3.0.5/.git:
	#@devtools/refresh-submodules.sh ext/openssl-3.0.5
.PHONY:openssl
openssl:ext/openssl-3.0.5/.git## 	openssl

cmake:## 	cmake .
##cmake
	$(MAKE) submodules || ls deps && ls ext && ls nips && echo ""
	cmake -S . -B build && cd build && make all
.PHONY:ext/openssl
ext/openssl:
##ext/openssl
	cd $(PWD)/$@-3.0.5 && \
		./Configure \
		--prefix=/usr/local/ssl \
		--openssldir=/usr/local/ssl \
		'-Wl,-rpath,$(LIBRPATH)' && $(MAKE) all
.PHONY:openssl
openssl:ext/openssl cmake## 	openssl
##openssl

checkbrew:## 	checkbrew
ifeq ($(HOMEBREW),)
	@/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
else
	## for wider macos support we use qt@5 qwt-qt5
	## qt qwt for qt@6 support
	## ccmake . for qt version options
	@type -P brew && brew install wxWidgets openssl@3.0 gettext qt@5 qwt-qt5
endif

tag:## 	git tag & git push
tags:tag
##tag
##	git tag v$(VERSION)-$(OS)-$(OS_VERSION)-$(ARCH)-$(shell date +%s)
	@git tag v$(VERSION)-$(OS)-$(OS_VERSION)-$(ARCH)-$(shell date +%s)
	@git push -f --tags || echo "unable to push tags..."

-include gnostr.mk
-include venv.mk
-include act.mk
-include clean.mk

# vim: set noexpandtab:
# vim: set setfiletype make
