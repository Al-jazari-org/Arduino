SKETCH := .
PORT ?= /dev/ttyUSB0
PLATFORM :=
PACKAGE :=
PLATFORM_CLEAN :=
INTERNAL := false
PARTITION :=
PARTITION_OFFSET :=
PARTITION_SIZE :=
BOARD :=
BOARD_ARGS :=
PLATFORM_DIR :=
MKSPIFFS :=
ESPTOOL :=
CACHE :=
FOLDER :=

define parse_sketch

$(eval PLATFORM = $(shell yq -r '.profiles[.default_profile].platforms[0].[]' ./sketch.yaml))
$(eval PLATFORM_CLEAN = $(shell echo "${PLATFORM}" | tr -d '\(\)' | tr ' :' '_'))
$(eval BOARD = $(shell yq -r '.profiles[.default_profile].fqbn' ./sketch.yaml ))
$(eval BOARD_ARGS = $(shell echo "$(BOARD)" | grep -oP '[^:]*=[^:]*$$'))
$(eval BOARD = $(shell echo "$(BOARD)" | sed 's/:[^:]*=[^:]*$$//'))
$(eval CACHE = $(shell arduino-cli board details --json -b $(BOARD)))
$(eval PACKAGE = $(shell echo '$(CACHE)' | yq -r '.package.name'))

$(if  ($(findstring esp32:esp32:esp32,$(BOARD)),esp32:esp32:esp32),


	$(eval PARTITION = $(shell echo "$(BOARD_ARGS)" | grep -oP 'PartitionScheme=\K[^:]*'))
	$(eval PARTITION = $(shell echo '$(CACHE)'  | yq -r '.config_options[] | select(.option == "PartitionScheme" ) | .values[] | select(.value == "$(PARTITION)") | .value' | less))
	$(if $(PARTITION),,
		$(eval PARTITION = $(shell echo '$(CACHE)'  | yq -r '.config_options[] | select(.option == "PartitionScheme" ) | .values[0].value'))
		)
	$(eval PLATFORM_DIR = $(shell find $(HOME)/.arduino15/internal -type d -name "$(PLATFORM_CLEAN)_*"))
	$(if $(PLATFORM_DIR),
		$(eval file = $(PLATFORM_DIR)/tools/partitions/$(PARTITION).csv)
		$(eval PARTITION_OFFSET := $(shell awk -F',' '$$1=="spiffs" {print $$4}' $(file)))
		$(eval PARTITION_SIZE := $(shell awk -F',' '$$1=="spiffs" {print $$5}'  $(file)))
		$(info $(PARTITION_OFFSET))
		$(info $(PARTITION_SIZE))

		$(eval mkspiffs_ver = $(shell cat $(PLATFORM_DIR)/installed.json | yq -r '.packages[] | select(.name == "esp32") | .platforms[] | select(.name == "esp32") | .toolsDependencies[] | select(.name == "mkspiffs") | .version' ))
		$(eval esptool_ver = $(shell cat $(PLATFORM_DIR)/installed.json | yq -r '.packages[] | select(.name == "esp32") | .platforms[] | select(.name == "esp32") | .toolsDependencies[] | select(.name == "esptool_py") | .version' ))

		$(info $(mkspiffs_ver))
		$(info $(esptool_ver))
		$(eval MKSPIFFS = $(shell find $(HOME)/.arduino15/internal -type d -name "$(PACKAGE)_mkspiffs_$(mkspiffs_ver)_*")/mkspiffs)
		$(eval ESPTOOL = $(shell find $(HOME)/.arduino15/internal -type d -name "$(PACKAGE)_esptool_py_$(esptool_ver)_*")/esptool.py)
		,

		$(eval PLATFORM_DIR = $(shell find $(HOME)/.arduino15/packages -type d -name "$(PACKAGE)"))
		# TBD
		)
	)



endef

define parse_esp32

endef

.PHONY: build upload all clean

all: build upload

build:
	arduino-cli compile $(SKETCH) --build-path ./build

upload:
	arduino-cli upload  $(SKETCH) -p $(PORT) --build-path ./build

monitor:
	arduino-cli monitor -p $(PORT)

mkspiffs:
	$(call parse_sketch)
	$(MKSPIFFS) -c $(FOLDER) -b 4096 -p 256 -s $(PARTITION_SIZE) spiffs.bin

spiffs-upload:
	$(call parse_sketch)
	python3 $(ESPTOOL) --chip esp32 --port $(PORT) --baud=921600 write_flash $(PARTITION_OFFSET) spiffs.bin



clean:
	arduino-cli compile $(SKETCH) --clean

