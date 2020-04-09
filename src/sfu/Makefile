#
# make tasks for sfuserver.
#

# Best effort to prefer Python 2 executable since there are yet pending issues
# with gyp and Python3.
PYTHON ?= $(shell command -v python2 2> /dev/null || echo python)
ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
CORES ?= $(shell ${ROOT_DIR}/scripts/cpu_cores.sh || echo 4)
SFUSERVER_BUILDTYPE ?= Release
GULP = ../node_modules/.bin/gulp
LCOV = ./deps/lcov/bin/lcov
BEAR ?= bear
JQ ?= jq
SED ?= sed
DOCKER ?= docker

.PHONY:	\
	default test fuzzer xcode lint format bear tidy clean clean-all \
	docker-build docker-run

default:
ifeq ($(SFUSERVER_WORKER_BIN),)
	$(PYTHON) ./scripts/configure.py -R sfuserver
	$(MAKE) -j$(CORES) BUILDTYPE=$(SFUSERVER_BUILDTYPE) -C out
endif



xcode:
	$(PYTHON) ./scripts/configure.py --format=xcode

lint:
	$(GULP) lint:worker

format:
	$(GULP) format:worker

bear:
	$(MAKE) clean
	$(BEAR) -o compile_commands_template.tmp.json $(MAKE)
	$(JQ) 'del(.[] | select(.file == "$(PWD)/src/Utils/IP.cpp"))' compile_commands_template.tmp.json > compile_commands_template.json
	$(RM) -f compile_commands_template.tmp.json
	$(SED) -i "s|$(PWD)|PATH|g" compile_commands_template.json

tidy:
	$(SED) "s|PATH|$(PWD)|g" compile_commands_template.json > compile_commands.json
	$(PYTHON) ./scripts/clang-tidy.py \
		-clang-tidy-binary=../node_modules/.bin/clang-tidy \
		-clang-apply-replacements-binary=../node_modules/.bin/clang-apply-replacements \
		-header-filter='(Channel/**/*.hpp|DepLibSRTP.hpp|base/application.h|DepLibWebRTC.hpp|DepOpenSSL.hpp|DepUsrSCTP.hpp|SfuServerError.hpp|RTC/**/*.hpp|Settings.hpp|Utils.hpp|Worker.hpp|common.hpp|handles/**/*.hpp)' \
		-p=. \
		-j=2 \
		-checks=$(SFUSERVER_TIDY_CHECKS) \
		-quiet

clean:
	$(RM) -rf out/Release/sfuserver
	$(RM) -rf out/Release/obj.target/sfuserver

	$(RM) -rf out/Debug/sfuserver
	$(RM) -rf out/Debug/obj.target/sfuserver


clean-all:
	$(RM) -rf out
	$(RM) -rf worker/sfuserver.xcodeproj
	$(RM) -rf worker/sfuserver-test.xcodeproj
	$(RM) -rf worker/deps/*/*.xcodeproj
	$(RM) -rf worker/libwebrtc/*/*.xcodeproj

