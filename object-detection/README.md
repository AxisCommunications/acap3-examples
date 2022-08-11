*Copyright (C) 2021, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# Object Detection Example

## Overview

This example focuses on the application of object detection on an Axis camera equipped with an Edge TPU, but can also be easily configured to run on CPU or ARTPEC8 cameras (DLPU). A pretrained model [MobileNet SSD v2 (COCO)] is used to detect the location of 90 types of different objects. The model is downloaded through the dockerfile from the google-coral repository. The detected objects are saved in /tmp folder for further usage.

## Prerequisites

- Axis camera equipped with an [Edge TPU](https://coral.ai/docs/edgetpu/faq/)
- [Docker](https://docs.docker.com/get-docker/)

## Quickstart

The following instructions can be executed to simply run the example.

1. Compile the ACAP application:

    ```sh
    docker build --build-arg ARCH=<ARCH> --build-arg CHIP=<CHIP> --tag obj_detect:1.0 .
    docker cp $(docker create obj_detect:1.0):/opt/app ./build
    ```

    where the values are found:
    - \<CHIP\> is the chip type. Supported values are *artpec8*, *cpu* and *edgetpu*.
    - \<ARCH\> is the architecture. Supported values are armv7hf (default) and aarch64

2. Find the ACAP application `.eap` file

    ```sh
    build/object_detection_app_1_0_0_<ARCH>.eap
    ```

3. Install and start the ACAP application on your camera through the camera web GUI

4. SSH to the camera

5. View its log to see the ACAP application output:

    ```sh
    tail -f /var/volatile/log/info.log | grep object_detection
    ```

## Designing the application

The whole principle is similar to the [tensorflow-to-larod](../tensorflow-to-larod). In this example, the original video has a resolution of 1920x1080, while the input size of MobileNet SSD COCO requires a input size of 300x300, so we set up two different streams, one is for MobileNet model, another is used to crop a higher resolution jpg image.

### Setting up the MobileNet Stream

There are two methods used to obtain a proper resolution. The [chooseStreamResolution](app/imgprovider.c#L221) method is used to select the smallest stream and assign them into streamWidth and streamHeight.

```c
unsigned int streamWidth = 0;
unsigned int streamHeight = 0;
chooseStreamResolution(args.width, args.height, &streamWidth, &streamHeight);
```

Then the [createImgProvider](app/imgprovider.c#L95) method is used to return an ImgProvider with the selected [output format](https://www.axis.com/techsup/developer_doc/acap3/3.5/api/vdostream/html/vdo-types_8h.html#a5ed136c302573571bf325c39d6d36246).

```c
provider = createImgProvider(streamWidth, streamHeight, 2, VDO_FORMAT_YUV);
```

#### Setting up the Crop Stream

The original resolution args.raw_width x args.raw_height is used to crop a higher resolution image.

```c
provider_raw = createImgProvider(args.raw_width, args.raw_height, 2, VDO_FORMAT_YUV);
```

#### Setting up the larod interface

Then similar with [tensorflow-to-larod](../tensorflow-to-larod), the [larod](https://www.axis.com/techsup/developer_doc/acap3/3.5/api/larod/html/index.html) interface needs to be set up. The [setupLarod](app/object_detection.c#L236) method is used to create a conncection to larod and select the hardware to use the model.

```c
int larodModelFd = -1;
larodConnection* conn = NULL;
larodModel* model = NULL;
setupLarod(args.chip, larodModelFd, &conn, &model);
```

The [createAndMapTmpFile](app/object_detection.c#L173) method is used to create temporary files to store the input and output tensors.

```c
char CONV_INP_FILE_PATTERN[] = "/tmp/larod.in.test-XXXXXX";
char CONV_OUT1_FILE_PATTERN[] = "/tmp/larod.out1.test-XXXXXX";
char CONV_OUT2_FILE_PATTERN[] = "/tmp/larod.out2.test-XXXXXX";
char CONV_OUT3_FILE_PATTERN[] = "/tmp/larod.out3.test-XXXXXX";
char CONV_OUT4_FILE_PATTERN[] = "/tmp/larod.out4.test-XXXXXX";
void* larodInputAddr = MAP_FAILED;
void* larodOutput1Addr = MAP_FAILED;
void* larodOutput2Addr = MAP_FAILED;
void* larodOutput3Addr = MAP_FAILED;
void* larodOutput4Addr = MAP_FAILED;
int larodInputFd = -1;
int larodOutput1Fd = -1;
int larodOutput2Fd = -1;
int larodOutput3Fd = -1;
int larodOutput4Fd = -1;

createAndMapTmpFile(CONV_INP_FILE_PATTERN,  args.width * args.height * CHANNELS, &larodInputAddr, &larodInputFd);
createAndMapTmpFile(CONV_OUT1_FILE_PATTERN, TENSOR1SIZE, &larodOutput1Addr, &larodOutput1Fd);
createAndMapTmpFile(CONV_OUT2_FILE_PATTERN, TENSOR2SIZE, &larodOutput2Addr, &larodOutput2Fd);
createAndMapTmpFile(CONV_OUT3_FILE_PATTERN, TENSOR3SIZE, &larodOutput3Addr, &larodOutput3Fd);
createAndMapTmpFile(CONV_OUT4_FILE_PATTERN, TENSOR4SIZE, &larodOutput4Addr, &larodOutput4Fd);
```

In terms of the crop part, another temporary file is created.

```c
char CROP_FILE_PATTERN[] = "/tmp/crop.test-XXXXXX";
void* cropAddr = MAP_FAILED;
int cropFd = -1;

createAndMapTmpFile(CROP_FILE_PATTERN, args.raw_width * args.raw_height * CHANNELS, &cropAddr, &cropFd);
```

The `larodCreateModelInputs` and `larodCreateModelOutputs` methods map the input and output tensors with the model.

```c
size_t numInputs = 0;
size_t numOutputs = 0;
inputTensors = larodCreateModelInputs(model, &numInputs, &error);
outputTensors = larodCreateModelOutputs(model, &numOutputs, &error);
```

The `larodSetTensorFd` method then maps each tensor to the corresponding file descriptor to allow IO.

```c
larodSetTensorFd(inputTensors[0], larodInputFd, &error);
larodSetTensorFd(outputTensors[0], larodOutput1Fd, &error);
larodSetTensorFd(outputTensors[1], larodOutput2Fd, &error);
larodSetTensorFd(outputTensors[2], larodOutput3Fd, &error);
larodSetTensorFd(outputTensors[3], larodOutput4Fd, &error);
```

Finally, the `larodCreateInferenceRequest` method creates an inference request to use the model.

```c
infReq = larodCreateInferenceRequest(model, inputTensors, numInputs, outputTensors, numOutputs, &error);
```

#### Fetching a frame and performing inference

By using the `getLastFrameBlocking` method, a  buffer containing the latest image is retrieved from the `ImgProvider` created earlier. Then `vdo_buffer_get_data` method is used to extract NV12 data from the buffer.

```c
VdoBuffer* buf = getLastFrameBlocking(provider);
uint8_t* nv12Data = (uint8_t*) vdo_buffer_get_data(buf);
```

Axis cameras output images on the NV12 YUV format. As this is not normally used as input format to deep learning models,
conversion to e.g., RGB might be needed. This can be done using ```libyuv```. However, if performance is a primary objective, training the model to use the YUV format directly should be considered, as each frame conversion takes a few milliseconds.  To convert the NV12 stream to RGB, the `convertCropScaleU8yuvToRGB` from `imgconverter` is used, which in turn uses the `libyuv` library.

```c
convertCropScaleU8yuvToRGB(nv12Data, streamWidth, streamHeight, (uint8_t*) larodInputAddr, args.width, args.height);
```

In terms of the frame used to crop the detected objects, there is no need to scale, so `convertU8yuvToRGBlibYuv` method is used.

```c
VdoBuffer* buf_hq = getLastFrameBlocking(provider_raw);
uint8_t* nv12Data_hq = (uint8_t*) vdo_buffer_get_data(buf_hq);

convertU8yuvToRGBlibYuv(args.raw_width, args.raw_height, nv12Data_hq, (uint8_t*) cropAddr);
```

By using the `larodRunInference` method, the predictions from the MobileNet are saved into the specified addresses.

```c
larodRunInference(conn, infReq, &error);
```

There are four outputs from the Object Detection model, and each object's location are described in the form of \[top, left, bottom, right\].

```c
float* locations = (float*) larodOutput1Addr;
float* classes = (float*) larodOutput2Addr;
float* scores = (float*) larodOutput3Addr;
float* numberofdetections = (float*) larodOutput4Addr;
```

If the score is higher than a threshold `args.threshold/100.0`, the results are outputted by the `syslog` function, and the object is cropped and saved into jpg form by `crop_interleaved`, `set_jpeg_configuration`, `buffer_to_jpeg`, `jpeg_to_file` methods.

```c
syslog(LOG_INFO, "Object %d: Classes: %s - Scores: %f - Locations: [%f,%f,%f,%f]",
i, class_name[(int) classes[i]], scores[i], top, left, bottom, right);

unsigned char* crop_buffer = crop_interleaved(cropAddr, args.raw_width, args.raw_height, CHANNELS,
                                          crop_x, crop_y, crop_w, crop_h);

buffer_to_jpeg(crop_buffer, &jpeg_conf, &jpeg_size, &jpeg_buffer);

jpeg_to_file(file_name, jpeg_buffer, jpeg_size);
```

## Building the application

An ACAP application contains a manifest file defining the package configuration.
The file is named `manifest.json.<CHIP>` and can be found in the [app](app)
directory. The Dockerfile will depending on the chip type(see below) copy the
file to the required name format `manifest.json`. The noteworthy attribute for
this tutorial is the `runOptions` attribute which allows arguments to be given
to the application and here is handled by the `argparse` lib. The
argument order, defined by [app/argparse.c](app/argparse.c), is `<model_path
input_resolution_width input_resolution_height output_size_in_bytes
raw_video_resolution_width raw_video_resolution_height threshold>`.

In the Dockerfile a `.tflite` model file corresponding to the chosen chip is
downloaded and added to the ACAP application via the -a flag in the
`acap-build` command.

The application is built to specification by the `Makefile` and `manifest.json`
in the [app](app) directory. Standing in the application direcory, run:

```sh
docker build --build-arg ARCH=<ARCH> --build-arg CHIP=<CHIP> --tag obj_detect:1.0 .
docker cp $(docker create obj_detect:1.0):/opt/app ./build
```

where the parameters are:

- \<CHIP\> is the chip type. Supported values are *artpec8*, *cpu* and *edgetpu*.
- \<ARCH\> is the architecture. Supported values are armv7hf (default) and aarch64

> N.b. The selected architecture and chip must match the targeted device.

The installable `.eap` file is found under:

```sh
build/object_detection_app_1_0_0_<ARCH>.eap
```

## Installing the application

To install an ACAP application, the `.eap` file in the `build` directory needs
to be uploaded to the camera and installed. This can be done through the camera
GUI. Then go to your camera -> Settings -> Apps -> Add -> Browse to the `.eap`
file and press Install.

## Running the application

In the Apps view of the camera, press the icon for your ACAP application. A
window will pop up which allows you to start the application. Press the Start
icon to run the algorithm.

With the algorithm started, we can view the output by either pressing `App log`
in the same window, or connect with SSH into the device and view the log with
the following command:

```sh
tail -f /var/volatile/log/info.log | grep object_detection
```

There are four outputs from MobileNet SSD v2 (COCO) model. The number of detections, cLasses, scores, and locations are shown as below. The four location numbers stand for [top, left, bottom, right]. By the way, currently the saved images will be overwritten continuously, so those saved images might not all from the detections of the last frame, if the number of detections is less than previous detection numbers.

```sh
[ INFO    ] object_detection[645]: Object 1: Classes: 2 car - Scores: 0.769531 - Locations: [0.750146,0.086451,0.894765,0.299347]
[ INFO    ] object_detection[645]: Object 2: Classes: 2 car - Scores: 0.335938 - Locations: [0.005453,0.101417,0.045346,0.144171]
[ INFO    ] object_detection[645]: Object 3: Classes: 2 car - Scores: 0.308594 - Locations: [0.109673,0.005128,0.162298,0.050947]
```

The detected objects with a score higher than a threshold are saved into /tmp folder in .jpg form as well.

## License

**[Apache License 2.0](../LICENSE)**
