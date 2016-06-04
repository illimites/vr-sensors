DOWNLOAD_DIR = download
BIN_DIR      = bin
ARDUPI_DIR   = arduPi
EHEALTH_DIR  = eHealth
BCM_DIR      = $(ARDUPI_DIR)/bcm2835-1.46
ARDUPI_ZIP   = cooking/arduPi
EHEALTH_ZIP  = eHealth_raspberrypi_v2.4

EHEALTH_URL = https://www.cooking-hacks.com/media/cooking/images/documentation/e_health_v2/eHealth_raspberrypi_v2.4.zip
ARDUPI_URL  = https://www.cooking-hacks.com/media/cooking/images/documentation/raspberry_arduino_shield/raspberrypi2.zip

# FIXME: arduPi.cpp won't compile on ArchLinux without -fpermissive
CPP_FLAGS = -fpermissive

PROGRAMS = $(BIN_DIR)/sensor_reader
EXAMPLES = $(BIN_DIR)/ecg $(BIN_DIR)/galvanic $(BIN_DIR)/pulsioximeter

.PHONY: clean distclean examples

all: $(PROGRAMS)

examples: $(EXAMPLES)

$(BIN_DIR)/sensor_reader: $(ARDUPI_DIR)/arduPi.o $(EHEALTH_DIR)/eHealth.o sensor_reader.o
	mkdir -p $(BIN_DIR)
	g++ -o $@ $^ -L "$(BCM_DIR)/src" -l bcm2835 -l rt -lpthread $(CPP_FLAGS)

$(EXAMPLES): $(BIN_DIR)/%: $(ARDUPI_DIR)/arduPi.o $(EHEALTH_DIR)/eHealth.o $(EHEALTH_DIR)/examples/%.o
	mkdir -p $(BIN_DIR)
	g++ -o $@ $^ -L "$(BCM_DIR)/src" -l bcm2835 -l rt -lpthread $(CPP_FLAGS)

.cpp.o:
	g++ -o $@ -c $< -I"$(EHEALTH_DIR)" -I "$(ARDUPI_DIR)" -I "$(BCM_DIR)/src" $(CPP_FLAGS)

$(DOWNLOAD_DIR)/arduPi.zip:
	mkdir -p $(DOWNLOAD_DIR)
	wget $(ARDUPI_URL) --output-document $(DOWNLOAD_DIR)/arduPi.zip

$(DOWNLOAD_DIR)/eHealth.zip:
	mkdir -p $(DOWNLOAD_DIR)
	wget $(EHEALTH_URL) --output-document $(DOWNLOAD_DIR)/eHealth.zip

$(ARDUPI_DIR)/arduPi.cpp: download/arduPi.zip
	cd $(DOWNLOAD_DIR); unzip arduPi.zip
	mv --no-target-directory $(DOWNLOAD_DIR)/$(ARDUPI_ZIP) $(ARDUPI_DIR)
	chmod +x $(BCM_DIR)/configure
	cd $(BCM_DIR); ./configure
	cd $(BCM_DIR)/src; $(MAKE) $(MFLAGS)
	touch --no-create $(ARDUPI_DIR)/arduPi.cpp

$(EHEALTH_DIR)/eHealth.cpp: download/eHealth.zip
	cd $(DOWNLOAD_DIR); unzip eHealth.zip
	mv --no-target-directory $(DOWNLOAD_DIR)/$(EHEALTH_ZIP) $(EHEALTH_DIR)

	# Update creation date so that the .cpp is newer than the .zip.
	# Otherwise make will always want tu rebuild this target.
	touch --no-create $(EHEALTH_DIR)/eHealth.cpp

clean:
	rm -rf $(DOWNLOAD_DIR) *.o

distclean: clean
	rm -rf $(ARDUPI_DIR) $(EHEALTH_DIR) $(BIN_DIR)
