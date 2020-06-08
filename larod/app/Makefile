DEPS = liblarod
CFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config \
            --cflags $(DEPS))
LDLIBS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config \
            --libs $(DEPS))
CFLAGS += -Wall -Wextra -Werror

.PHONY: all
all: larod_simple_app model/mobilenet_v1_1.0_224_quant.larod

model/mobilenet_v1_1.0_224_quant.larod:
	mkdir -p model
	curl -o model/mobilenet_v1_tflite.zip https://storage.googleapis.com/download.tensorflow.org/models/tflite/mobilenet_v1_1.0_224_quant_and_labels.zip
	unzip -d model model/mobilenet_v1_tflite.zip
	larod-convert.py tflite model/mobilenet_v1_1.0_224_quant.tflite

.PHONY: clean
clean:
	-mv package.conf.orig package.conf
	rm -f larod_simple_app *.o *.eap *_LICENSE.txt
	rm -rf model
