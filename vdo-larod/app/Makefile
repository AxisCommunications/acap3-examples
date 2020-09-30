PROG1	= vdo_larod
OBJS1	= $(PROG1).c argparse.c imgconverter.c imgprovider.c
PROGS	= $(PROG1)

PKGS = gio-2.0 vdostream gio-unix-2.0 liblarod

CFLAGS  += -Iinclude

LDLIBS  += -lyuv
LDFLAGS += -L./lib -Wl,-rpath,'$$ORIGIN/lib'

CFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags $(PKGS))
LDLIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs $(PKGS))

CFLAGS += -Wall

SDK_TARGET_LIBS=$(SDKTARGETSYSROOT)/usr/lib

SHLIB_DIR = ./lib

all:	$(PROGS) \
		model/mobilenet_v2_1.0_224_quant.larod \
		model/mobilenet_v2_1.0_224_quant_edgetpu.larod \
		label/imagenet_labels.txt \
		libscopy

$(PROG1): $(OBJS1)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

model/mobilenet_v2_1.0_224_quant.larod:
	mkdir -p model
	curl -o model/mobilenet_v2_1.0_224_quant.tgz http://download.tensorflow.org/models/tflite_11_05_08/mobilenet_v2_1.0_224_quant.tgz
	tar -xvf model/mobilenet_v2_1.0_224_quant.tgz -C model
	larod-convert.py tflite model/mobilenet_v2_1.0_224_quant.tflite
	rm -f model/*.tgz model/*.tflite model/*.pb* model/*.ckpt* model/*.meta model/*.txt

model/mobilenet_v2_1.0_224_quant_edgetpu.larod:
	mkdir -p model
	curl -L -o model/mobilenet_v2_1.0_224_quant_edgetpu.tflite https://github.com/google-coral/edgetpu/raw/master/test_data/mobilenet_v2_1.0_224_quant_edgetpu.tflite
	larod-convert.py tflite model/mobilenet_v2_1.0_224_quant_edgetpu.tflite
	rm -f model/*.tflite

label/imagenet_labels.txt:
	mkdir -p label
	curl -L -o label/imagenet_labels.txt https://github.com/google-coral/edgetpu/raw/master/test_data/imagenet_labels.txt

libscopy:
	mkdir -p $(SHLIB_DIR)
	cp $(SDK_TARGET_LIBS)/libyuv.so             $(SHLIB_DIR)/libyuv.so
	cp $(SDK_TARGET_LIBS)/libyuv.so.1           $(SHLIB_DIR)/libyuv.so.1
	cp $(SDK_TARGET_LIBS)/libyuv.so.1.0         $(SHLIB_DIR)/libyuv.so.1.0

clean:
	rm -f $(PROGS) *.o *.eap lib/* include/* model/* label/*
