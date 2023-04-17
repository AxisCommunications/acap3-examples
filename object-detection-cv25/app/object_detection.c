/**
 * Copyright (C) 2022 Axis Communications AB, Lund, Sweden
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 /**
  * - object_detection -
  *
  * This application loads a larod model which takes an image as input and
  * outputs values corresponding to the class, score and location of detected
  * objects in the image.
  *
  * The application expects eight arguments on the command line in the
  * following order: MODEL WIDTH HEIGHT PADDING QUALITY RAW_WIDTH RAW_HEIGHT
  * THRESHOLD LABELSFILE NUMLABELS NUMDETECTIONS ANCHORSFILE.
  *
  * First argument, MODEL, is a string describing path to the model.
  *
  * Second argument, WIDTH, is an integer for the input width.
  *
  * Third argument, HEIGHT, is an integer for the input height.
  *
  * Fourth argument, PADDING, is an integer for the width padding.
  *
  * Fifth argument, QUALITY, is an integer for the desired jpeg quality.
  *
  * Sixth argument, RAW_WIDTH, is an integer for the camera's width resolution.
  *
  * Seventh argument, RAW_HEIGHT, is an integer for the camera's height resolution.
  *
  * Eighth argument, THRESHOLD, is an integer ranging from 0 to 100 to select good detections.
  *
  * Ninth argument, LABELSFILE, is a string describing the path to the label file.
  *
  * Tenth argument, NUMLABELS, is an integer that defines the number of classes.
  *
  * Eleventh argument, NUMDETECTIONS, is an integer that defines the number of detections.
  *
  * Twelfth  argument, ANCHORSFILE, is a string describing path to the anchors.
  */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include "argparse.h"
#include "imgprovider.h"
#include "imgutils.h"
#include "larod.h"
#include "vdo-frame.h"
#include "vdo-types.h"
#include "postprocessing.h"

/**
 * @brief Free up resources held by an array of labels.
 *
 * @param labels An array of label string pointers.
 * @param labelFileBuffer Heap buffer containing the actual string data.
 */
void freeLabels(char** labelsArray, char* labelFileBuffer) {
    free(labelsArray);
    free(labelFileBuffer);
}

static void padImageWidth(uint8_t* srcimage, uint8_t* dstimage, unsigned int width, unsigned int height, unsigned int padding){
    for (size_t i = 0; i < height; i++){
        for (size_t j = 0; j < width; j++){
            dstimage[i*(width+padding) + j] = srcimage[i*width + j];
        }
    }
}

/**
 * @brief Reads a file of labels into an array.
 *
 * An array filled by this function should be freed using freeLabels.
 *
 * @param labelsPtr Pointer to a string array.
 * @param labelFileBuffer Pointer to the labels file contents.
 * @param labelsPath String containing the path to the labels file to be read.
 * @param numLabelsPtr Pointer to number which will store number of labels read.
 * @return False if any errors occur, otherwise true.
 */
static bool parseLabels(char*** labelsPtr, char** labelFileBuffer, char *labelsPath,
                 size_t* numLabelsPtr) {
    // We cut off every row at 60 characters.
    const size_t LINE_MAX_LEN = 60;
    bool ret = false;
    char* labelsData = NULL;  // Buffer containing the label file contents.
    char** labelArray = NULL; // Pointers to each line in the labels text.

    struct stat fileStats = {0};
    if (stat(labelsPath, &fileStats) < 0) {
        syslog(LOG_ERR, "%s: Unable to get stats for label file %s: %s", __func__,
               labelsPath, strerror(errno));
        return false;
    }

    // Sanity checking on the file size - we use size_t to keep track of file
    // size and to iterate over the contents. off_t is signed and 32-bit or
    // 64-bit depending on architecture. We just check toward 10 MByte as we
    // will not encounter larger label files and both off_t and size_t should be
    // able to represent 10 megabytes on both 32-bit and 64-bit systems.
    if (fileStats.st_size > (10 * 1024 * 1024)) {
        syslog(LOG_ERR, "%s: failed sanity check on labels file size", __func__);
        return false;
    }

    int labelsFd = open(labelsPath, O_RDONLY);
    if (labelsFd < 0) {
        syslog(LOG_ERR, "%s: Could not open labels file %s: %s", __func__, labelsPath,
               strerror(errno));
        return false;
    }

    size_t labelsFileSize = (size_t) fileStats.st_size;
    // Allocate room for a terminating NULL char after the last line.
    labelsData = malloc(labelsFileSize + 1);
    if (labelsData == NULL) {
        syslog(LOG_ERR, "%s: Failed allocating labels text buffer: %s", __func__,
               strerror(errno));
        goto end;
    }

    ssize_t numBytesRead = -1;
    size_t totalBytesRead = 0;
    char* fileReadPtr = labelsData;
    while (totalBytesRead < labelsFileSize) {
        numBytesRead =
            read(labelsFd, fileReadPtr, labelsFileSize - totalBytesRead);

        if (numBytesRead < 1) {
            syslog(LOG_ERR, "%s: Failed reading from labels file: %s", __func__,
                   strerror(errno));
            goto end;
        }
        totalBytesRead += (size_t) numBytesRead;
        fileReadPtr += numBytesRead;
    }

    // Now count number of lines in the file - check all bytes except the last
    // one in the file.
    size_t numLines = 0;
    for (size_t i = 0; i < (labelsFileSize - 1); i++) {
        if (labelsData[i] == '\n') {
            numLines++;
        }
    }

    // We assume that there is always a line at the end of the file, possibly
    // terminated by newline char. Either way add this line as well to the
    // counter.
    numLines++;

    labelArray = malloc(numLines * sizeof(char*));
    if (!labelArray) {
        syslog(LOG_ERR, "%s: Unable to allocate labels array: %s", __func__,
               strerror(errno));
        ret = false;
        goto end;
    }

    size_t labelIdx = 0;
    labelArray[labelIdx] = labelsData;
    labelIdx++;
    for (size_t i = 0; i < labelsFileSize; i++) {
        if (labelsData[i] == '\n') {
            // Register the string start in the list of labels.
            labelArray[labelIdx] = labelsData + i + 1;
            labelIdx++;
            // Replace the newline char with string-ending NULL char.
            labelsData[i] = '\0';
        }
    }

    // If the very last byte in the labels file was a new-line we just
    // replace that with a NULL-char. Refer previous for loop skipping looking
    // for new-line at the end of file.
    if (labelsData[labelsFileSize - 1] == '\n') {
        labelsData[labelsFileSize - 1] = '\0';
    }

    // Make sure we always have a terminating NULL char after the label file
    // contents.
    labelsData[labelsFileSize] = '\0';

    // Now go through the list of strings and cap if strings too long.
    for (size_t i = 0; i < numLines; i++) {
        size_t stringLen = strnlen(labelArray[i], LINE_MAX_LEN);
        if (stringLen >= LINE_MAX_LEN) {
            // Just insert capping NULL terminator to limit the string len.
            *(labelArray[i] + LINE_MAX_LEN + 1) = '\0';
        }
    }

    *labelsPtr = labelArray;
    *numLabelsPtr = numLines;
    *labelFileBuffer = labelsData;

    ret = true;

end:
    if (!ret) {
        freeLabels(labelArray, labelsData);
    }
    close(labelsFd);

    return ret;
}

/// Set by signal handler if an interrupt signal sent to process.
/// Indicates that app should stop asap and exit gracefully.
volatile sig_atomic_t stopRunning = false;

/**
 * @brief Invoked on SIGINT. Makes app exit cleanly asap if invoked once, but
 * forces an immediate exit without clean up if invoked at least twice.
 *
 * @param sig What signal has been sent. Will always be SIGINT.
 */
void sigintHandler(int sig) {
    // Force an exit if SIGINT has already been sent before.
    if (stopRunning) {
        syslog(LOG_INFO, "Interrupted again, exiting immediately without clean up.");

        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Interrupted, starting graceful termination of app. Another "
            "interrupt signal will cause a forced exit.");

    // Tell the main thread to stop running inferences asap.
    stopRunning = true;
}


/**
 * @brief Creates a temporary fd and truncated to correct size and mapped.
 *
 * This convenience function creates temp files to be used for input and output.
 *
 * @param fileName Pattern for how the temp file will be named in file system.
 * @param fileSize How much space needed to be allocated (truncated) in fd.
 * @param mappedAddr Pointer to the address of the fd mapped for this process.
 * @param Pointer to the generated fd.
 * @return Positive errno style return code (zero means success).
 */
static bool createAndMapTmpFile(char* fileName, size_t fileSize, void** mappedAddr,
                         int* convFd) {
    syslog(LOG_INFO, "%s: Setting up a temp fd with pattern %s and size %zu", __func__,
            fileName, fileSize);

    int fd = mkstemp(fileName);
    if (fd < 0) {
        syslog(LOG_ERR, "%s: Unable to open temp file %s: %s", __func__, fileName,
                 strerror(errno));
        goto error;
    }

    // Allocate enough space in for the fd.
    if (ftruncate(fd, (off_t) fileSize) < 0) {
        syslog(LOG_ERR, "%s: Unable to truncate temp file %s: %s", __func__, fileName,
                 strerror(errno));
        goto error;
    }

    // Remove since we don't actually care about writing to the file system.
    if (unlink(fileName)) {
        syslog(LOG_ERR, "%s: Unable to unlink from temp file %s: %s", __func__,
                 fileName, strerror(errno));
        goto error;
    }

    // Get an address to fd's memory for this process's memory space.
    void* data =
        mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
        syslog(LOG_ERR, "%s: Unable to mmap temp file %s: %s", __func__, fileName,
                 strerror(errno));
        goto error;
    }

    *mappedAddr = data;
    *convFd = fd;

    return true;

error:
    if (fd >= 0) {
        close(fd);
    }

    return false;
}

/**
 * @brief Sets up and configures a connection to larod, and loads a model.
 *
 * Opens a connection to larod, which is tied to larodConn. After opening a
 * larod connection the chip specified by larodChip is set for the
 * connection. Then the model file specified by larodModelFd is loaded to the
 * chip, and a corresponding larodModel object is tied to model.
 *
 * larodChip Speficier for which larod chip to use.
 * @param larodModelFd Fd for a model file to load.
 * @param larodConn Pointer to a larod connection to be opened.
 * @param model Pointer to a larodModel to be obtained.
 * @return false if error has occurred, otherwise true.
 */
static bool setupLarod(const char* chipString, const int larodModelFd,
                       larodConnection** larodConn, larodModel** model) {
    larodError* error = NULL;
    larodConnection* conn = NULL;
    larodModel* loadedModel = NULL;
    bool ret = false;

    // Set up larod connection.
    if (!larodConnect(&conn, &error)) {
        syslog(LOG_ERR, "%s: Could not connect to larod: %s", __func__, error->msg);
        goto end;
    }

    // List available chip id:s
    size_t numDevices = 0;
    syslog(LOG_INFO, "Available chip IDs:");
    const larodDevice** devices;
    devices = larodListDevices(conn, &numDevices, &error);
    for (size_t i = 0; i < numDevices; ++i) {
            syslog(LOG_INFO, "%s: %s", "Chip", larodGetDeviceName(devices[i], &error));;
        }
    const larodDevice* dev = larodGetDevice(conn, chipString, 0, &error);
    loadedModel = larodLoadModel(conn, larodModelFd, dev, LAROD_ACCESS_PRIVATE,
                                 "object_detection", NULL, &error);
    if (!loadedModel) {
        syslog(LOG_ERR, "%s: Unable to load model: %s", __func__, error->msg);
        goto error;
    }
    *larodConn = conn;
    *model = loadedModel;

    ret = true;

    goto end;

error:
    if (conn) {
        larodDisconnect(&conn, NULL);
    }

end:
    if (error) {
        larodClearError(&error);
    }

    return ret;
}


/**
 * @brief Main function that starts a stream with different options.
 */
int main(int argc, char** argv) {
    // Hardcode to use three image "color" channels (eg. RGB).
    const unsigned int CHANNELS = 3;
    // Hardcode to set output bytes of four tensors from MobileNet V2 model.
    const unsigned int FLOATSIZE = 4;
    const unsigned int TENSOR1SIZE = 1917 *  4 * FLOATSIZE;
    const unsigned int TENSOR2SIZE = 1917 * 91 * FLOATSIZE;

    // Name patterns for the temp file we will create.
    char CONV_INP_FILE_PATTERN[] = "/tmp/larod.in.test-XXXXXX";
    char CONV_PP_FILE_PATTERN[] = "/tmp/larod.pp.test-XXXXXX";
    char CROP_FILE_PATTERN[] = "/tmp/crop.test-XXXXXX";
    char CONV_OUT1_FILE_PATTERN[] = "/tmp/larod.out1.test-XXXXXX";
    char CONV_OUT2_FILE_PATTERN[] = "/tmp/larod.out2.test-XXXXXX";

    bool ret = false;
    ImgProvider_t* provider = NULL;
    ImgProvider_t* provider_raw = NULL;
    larodError* error = NULL;
    larodConnection* conn = NULL;
    larodMap* ppMap = NULL;
    larodMap* cropMap = NULL;
    larodModel* ppModel = NULL;
    larodModel* model = NULL;
    larodTensor** ppInputTensors = NULL;
    size_t ppNumInputs = 0;
    larodTensor** ppOutputTensors = NULL;
    size_t ppNumOutputs = 0;
    larodTensor** inputTensors = NULL;
    size_t numInputs = 0;
    larodTensor** outputTensors = NULL;
    size_t numOutputs = 0;
    larodJobRequest* ppReq = NULL;
    larodJobRequest* infReq = NULL;
    void* ppInputAddr = MAP_FAILED;
    size_t outputBufferSize = 0;
    int ppInputFd = -1;
    void* larodInputAddr = MAP_FAILED;
    void* cropAddr = MAP_FAILED;
    void* larodOutput1Addr = MAP_FAILED;
    void* larodOutput2Addr = MAP_FAILED;
    int larodModelFd = -1;
    int larodInputFd = -1;
    int cropFd = -1;
    int larodOutput1Fd = -1;
    int larodOutput2Fd = -1;
    box* boxes = NULL;
    uint8_t* rgb_image = NULL;
    char** labels = NULL; // This is the array of label strings. The label
                          // entries points into the large labelFileData buffer.
    size_t numLabels = 0; // Number of entries in the labels array.
    char* labelFileData =
        NULL; // Buffer holding the complete collection of label strings.

    // Open the syslog to report messages for "object_detection"
    openlog("object_detection", LOG_PID|LOG_CONS, LOG_USER);

    args_t args;
    if (!parseArgs(argc, argv, &args)) {
        goto end;
    }

    const char* chipString = args.chip;
    const char* modelFile = args.modelFile;
    const char* labelsFile = args.labelsFile;
    const int inputWidth = args.width;
    const int inputHeight = args.height;
    const int rawWidth = args.raw_width;
    const int rawHeight = args.raw_height;
    const int threshold = args.threshold;
    const int quality = args.quality;
    const int numberOfDetections = args.numDetections; // number of detections
    const int numberOfClasses= args.numLabels; // number of classes
    const char* anchorFile = args.anchorsFile;
    const int padding = args.padding;


    syslog(LOG_INFO, "Starting %s", argv[0]);
    // Register an interrupt handler which tries to exit cleanly if invoked once
    // but exits immediately if further invoked.
    signal(SIGINT, sigintHandler);

    unsigned int streamWidth = 0;
    unsigned int streamHeight = 0;
    if (!chooseStreamResolution(inputWidth, inputHeight, &streamWidth,
                                &streamHeight)) {
        syslog(LOG_ERR, "%s: Failed choosing stream resolution", __func__);
        goto end;
    }

    syslog(LOG_INFO, "Creating VDO image provider and creating stream %d x %d",
           streamWidth, streamHeight);
    provider = createImgProvider(streamWidth, streamHeight, 2, VDO_FORMAT_YUV);
    if (!provider) {
      syslog(LOG_ERR, "%s: Could not create image provider", __func__);
        goto end;
    }

    syslog(LOG_INFO, "Creating VDO raw image provider and stream %d x %d",
            rawWidth, rawHeight);
    provider_raw = createImgProvider(rawWidth, rawHeight, 2, VDO_FORMAT_YUV);
    if (!provider_raw) {
      syslog(LOG_ERR, "%s: Could not create raw image provider", __func__);
    }

    syslog(LOG_INFO, "raw width=%d, raw height=%d ", rawWidth, rawHeight);
    unsigned int cropStreamWidth = 0;
    unsigned int cropStreamHeight = 0;
    if (!chooseStreamResolution(rawWidth, rawHeight, &cropStreamWidth,
                                &cropStreamHeight)) {
        syslog(LOG_ERR, "%s: Failed choosing crop stream resolution", __func__);
        goto end;
    }

    syslog(LOG_INFO, "Creating VDO crop image provider and creating stream %d x %d",
            streamWidth, streamHeight);
    
    // Calculate crop image
    // 1. The crop area shall fill the input image either horizontally or
    //    vertically.
    // 2. The crop area shall have the same aspect ratio as the output image.
    syslog(LOG_INFO, "Calculate crop image");
    float destWHratio = (float) inputWidth / (float) inputHeight;
    float cropW = (float) streamWidth;
    float cropH = cropW / destWHratio;
    if (cropH > (float) streamHeight) {
        cropH = (float) streamHeight;
        cropW = cropH * destWHratio;
    }
    unsigned int clipW = (unsigned int)cropW;
    unsigned int clipH = (unsigned int)cropH;
    unsigned int clipX = (streamWidth - clipW) / 2;
    unsigned int clipY = (streamHeight - clipH) / 2;
    syslog(LOG_INFO, "Crop VDO image X=%d Y=%d (%d x %d)", clipX, clipY, clipW, clipH);

    // Create preprocessing maps
    syslog(LOG_INFO, "Create preprocessing maps");
    ppMap = larodCreateMap(&error);
    if (!ppMap) {
        syslog(LOG_ERR, "Could not create preprocessing larodMap %s", error->msg);
        goto end;
    }
    if (!larodMapSetStr(ppMap, "image.input.format", "nv12", &error)) {
        syslog(LOG_ERR, "Failed setting preprocessing parameters: %s", error->msg);
        goto end;
    }
    if (!larodMapSetIntArr2(ppMap, "image.input.size", streamWidth, streamHeight, &error)) {
        syslog(LOG_ERR, "Failed setting preprocessing parameters: %s", error->msg);
        goto end;
    }
    if(chipString != "ambarella-cvflow"){
        if (!larodMapSetStr(ppMap, "image.output.format", "rgb-interleaved", &error)) {
            syslog(LOG_ERR, "Failed setting preprocessing parameters: %s", error->msg);
            goto end;
        }
    } else {
        if (!larodMapSetStr(ppMap, "image.output.format", "rgb-planar", &error)) {
            syslog(LOG_ERR, "Failed setting preprocessing parameters: %s", error->msg);
            goto end;
        }
    }
    if (!larodMapSetIntArr2(ppMap, "image.output.size", inputWidth, inputHeight, &error)) {
        syslog(LOG_ERR, "Failed setting preprocessing parameters: %s", error->msg);
        goto end;
    }
    cropMap = larodCreateMap(&error);
    if (!cropMap) {
        syslog(LOG_ERR, "Could not create preprocessing crop larodMap %s", error->msg);
        goto end;
    }
    if (!larodMapSetIntArr4(cropMap, "image.input.crop", clipX, clipY, clipW, clipH, &error)) {
        syslog(LOG_ERR, "Failed setting preprocessing parameters: %s", error->msg);
        goto end;
    }

    // Create larod models
    syslog(LOG_INFO, "Create larod models");
    larodModelFd = open(modelFile, O_RDONLY);
    if (larodModelFd < 0) {
        syslog(LOG_ERR, "Unable to open model file %s: %s", modelFile,
               strerror(errno));
        goto end;
    }


    syslog(LOG_INFO, "Setting up larod connection with chip %s, model %s and label file %s", chipString, modelFile, labelsFile);
    if (!setupLarod(chipString, larodModelFd, &conn, &model)) {
        goto end;
    }

    // Use libyuv as image preprocessing backend
    const char* larodLibyuvPP = "cpu-proc";
    const larodDevice* dev_pp;
    dev_pp = larodGetDevice(conn, larodLibyuvPP, 0, &error);
    ppModel = larodLoadModel(conn, -1, dev_pp, LAROD_ACCESS_PRIVATE, "", ppMap, &error);
    if (!ppModel) {
            syslog(LOG_ERR, "Unable to load preprocessing model with chip %s: %s", larodLibyuvPP, error->msg);
            goto end;
    } else {
           syslog(LOG_INFO, "Loading preprocessing model with chip %s", larodLibyuvPP);
    }
    syslog(LOG_INFO, "Creating temporary files and memmaps for inference input and "
            "output tensors");

    // Create input/output tensors
    syslog(LOG_INFO, "Create input/output tensors");
    ppInputTensors = larodCreateModelInputs(ppModel, &ppNumInputs, &error);
    if (!ppInputTensors) {
        syslog(LOG_ERR, "Failed retrieving input tensors: %s", error->msg);
        goto end;
    }
    ppOutputTensors = larodCreateModelOutputs(ppModel, &ppNumOutputs, &error);
    if (!ppOutputTensors) {
        syslog(LOG_ERR, "Failed retrieving output tensors: %s", error->msg);
        goto end;
    }
    inputTensors = larodCreateModelInputs(model, &numInputs, &error);
    if (!inputTensors) {
        syslog(LOG_ERR, "Failed retrieving input tensors: %s", error->msg);
        goto end;
    }

    outputTensors = larodCreateModelOutputs(model, &numOutputs, &error);
    if (!outputTensors) {
        syslog(LOG_ERR, "Failed retrieving output tensors: %s", error->msg);
        goto end;
    }



    // Determine tensor buffer sizes
    syslog(LOG_INFO, "Determine tensor buffer sizes");
    const larodTensorPitches* ppInputPitches = larodGetTensorPitches(ppInputTensors[0], &error);
    if (!ppInputPitches) {
        syslog(LOG_ERR, "Could not get pitches of tensor: %s", error->msg);
        goto end;
    }
    size_t yuyvBufferSize = ppInputPitches->pitches[0];
    const larodTensorPitches* outputPitches = larodGetTensorPitches(outputTensors[0], &error);
    if (!outputPitches) {
        syslog(LOG_ERR, "Could not get pitches of tensor: %s", error->msg);
        goto end;
    }
    outputBufferSize = outputPitches->pitches[0];

    // Allocate space for input tensor
    syslog(LOG_INFO, "Allocate memory for input/output buffers");
    if (!createAndMapTmpFile(CONV_INP_FILE_PATTERN,
                             (inputWidth + padding) * inputHeight * CHANNELS,
                             &larodInputAddr, &larodInputFd)) {
        goto end;
    }
    if (!createAndMapTmpFile(CONV_PP_FILE_PATTERN, yuyvBufferSize,
                             &ppInputAddr, &ppInputFd)) {
        goto end;
    }
    if (!createAndMapTmpFile(CROP_FILE_PATTERN,
                             rawWidth * rawHeight * CHANNELS,
                             &cropAddr, &cropFd)) {
        goto end;
    }
    if (!createAndMapTmpFile(CONV_OUT1_FILE_PATTERN, TENSOR1SIZE,
                             &larodOutput1Addr, &larodOutput1Fd)) {
        goto end;
    }
    if (!createAndMapTmpFile(CONV_OUT2_FILE_PATTERN, TENSOR2SIZE,
                             &larodOutput2Addr, &larodOutput2Fd)) {
        goto end;
    }

    // Connect tensors to file descriptors
    syslog(LOG_INFO, "Connect tensors to file descriptors");
    if (!larodSetTensorFd(ppInputTensors[0], ppInputFd, &error)) {
        syslog(LOG_ERR, "Failed setting input tensor fd: %s", error->msg);
        goto end;
    }
    if (!larodSetTensorFd(ppOutputTensors[0], larodInputFd, &error)) {
        syslog(LOG_ERR, "Failed setting input tensor fd: %s", error->msg);
        goto end;
    }

    syslog(LOG_INFO, "Set input tensors");
    if (!larodSetTensorFd(inputTensors[0], larodInputFd, &error)) {
        syslog(LOG_ERR, "Failed setting input tensor fd: %s", error->msg);
        goto end;
    }

    syslog(LOG_INFO, "Set output tensors");
    if (!larodSetTensorFd(outputTensors[0], larodOutput1Fd, &error)) {
        syslog(LOG_ERR, "Failed setting output tensor fd: %s", error->msg);
        goto end;
    }

    if (!larodSetTensorFd(outputTensors[1], larodOutput2Fd, &error)) {
        syslog(LOG_ERR, "Failed setting output tensor fd: %s", error->msg);
        goto end;
    }

    // Create job requests
    syslog(LOG_INFO, "Create job requests");
    ppReq = larodCreateJobRequest(ppModel, ppInputTensors, ppNumInputs,
        ppOutputTensors, ppNumOutputs, cropMap, &error);
    if (!ppReq) {
        syslog(LOG_ERR, "Failed creating preprocessing job request: %s", error->msg);
        goto end;
    }

    // App supports only one input/output tensor.
    infReq = larodCreateJobRequest(model, inputTensors, numInputs, outputTensors,
                                         numOutputs, NULL, &error);
    if (!infReq) {
        syslog(LOG_ERR, "Failed creating inference request: %s", error->msg);
        goto end;
    }

    if (labelsFile) {
        if (!parseLabels(&labels, &labelFileData, labelsFile,
                         &numLabels)) {
            syslog(LOG_ERR, "Failed creating parsing labels file");
            goto end;
        }
    }

    syslog(LOG_INFO, "Found %d input tensors and %d output tensors",(int) numInputs, (int) numOutputs);
    syslog(LOG_INFO, "Start fetching video frames from VDO");
    if (!startFrameFetch(provider)) {
        syslog(LOG_ERR, "Stuck in provider");
        goto end;
    }

    if (!startFrameFetch(provider_raw)) {
        syslog(LOG_ERR, "Stuck in provider raw");
        goto end;
    }

    // This contains the box coordinates and class scores for each detected object.
    boxes = (box*)malloc(sizeof(box) * numberOfDetections);
    rgb_image = (uint8_t*)calloc(sizeof(uint8_t), inputWidth * inputHeight * CHANNELS);

    while (true) {
        struct timeval startTs, endTs;
        unsigned int elapsedMs = 0;

        // Get latest frame from image pipeline.
        VdoBuffer* buf = getLastFrameBlocking(provider);
        if (!buf) {
            syslog(LOG_ERR, "buf empty in provider");
            goto end;
        }

        VdoBuffer* buf_hq = getLastFrameBlocking(provider_raw);
        if (!buf_hq) {
            syslog(LOG_ERR, "buf empty in provider raw");
            goto end;
        }

        // Get data from latest frame.
        uint8_t* nv12Data = (uint8_t*) vdo_buffer_get_data(buf);
        uint8_t* nv12Data_hq = (uint8_t*) vdo_buffer_get_data(buf_hq);

        // Covert image data from NV12 format to interleaved uint8_t RGB format.
        gettimeofday(&startTs, NULL);

        memcpy(ppInputAddr, nv12Data, yuyvBufferSize);
        if (!larodRunJob(conn, ppReq, &error)) {
            syslog(LOG_ERR, "Unable to run job to preprocess model: %s (%d)",
                   error->msg, error->code);
            goto end;
        }
        padImageWidth(rgb_image, larodInputAddr, inputWidth, inputHeight, padding);

        gettimeofday(&endTs, NULL);

        elapsedMs = (unsigned int) (((endTs.tv_sec - startTs.tv_sec) * 1000) +
                                    ((endTs.tv_usec - startTs.tv_usec) / 1000));
        syslog(LOG_INFO, "Converted image in %u ms", elapsedMs);

        // Since larodOutputAddr points to the beginning of the fd we should
        // rewind the file position before each job.
        if (lseek(larodOutput1Fd, 0, SEEK_SET) == -1) {
            syslog(LOG_ERR, "Unable to rewind output file position: %s",
                   strerror(errno));
            goto end;
        }

        if (lseek(larodOutput2Fd, 0, SEEK_SET) == -1) {
            syslog(LOG_ERR, "Unable to rewind output file position: %s",
                     strerror(errno));

            goto end;
        }

        gettimeofday(&startTs, NULL);
        if (!larodRunJob(conn, infReq, &error)) {
            syslog(LOG_ERR, "Unable to run inference on model %s: %s (%d)",
                   labelsFile, error->msg, error->code);
            goto end;
        }
        gettimeofday(&endTs, NULL);

        elapsedMs = (unsigned int) (((endTs.tv_sec - startTs.tv_sec) * 1000) +
                                    ((endTs.tv_usec - startTs.tv_usec) / 1000));
        syslog(LOG_INFO, "Ran inference for %u ms", elapsedMs);

        float* locations = (float*) larodOutput1Addr;
        float* classes = (float*) larodOutput2Addr;


        // hyperparameters depend on the model used. For the model used in this example
        // the values come from the config file used to train the model.
        // https://github.com/tensorflow/models/blob/master/research/object_detection/samples/configs/ssd_mobilenet_v2_coco.config#L11
        int confidenceThreshold = threshold/100.0;
        int iouThreshold = 0.5;
        int yScale = 10;
        int xScale = 10;
        int hScale = 5;
        int wScale = 5;

        gettimeofday(&startTs, NULL);
        // postprocessing the output of the network. This will fill the boxes array.
        postProcessing(locations,classes,numberOfDetections,anchorFile,numberOfClasses,confidenceThreshold,iouThreshold,yScale,xScale,hScale,wScale,boxes);
        gettimeofday(&endTs, NULL);

        elapsedMs = (unsigned int) (((endTs.tv_sec - startTs.tv_sec) * 1000) +
                                    ((endTs.tv_usec - startTs.tv_usec) / 1000));
        syslog(LOG_INFO, "Postprocesing in %u ms", elapsedMs);
        for (int i = 0; i < numberOfDetections; i++){


            float top = boxes[i].y_min;
            float left = boxes[i].x_min;
            float bottom = boxes[i].y_max;
            float right = boxes[i].x_max;

            unsigned int crop_x = left * rawWidth + (rawWidth- rawHeight)/2;
            unsigned int crop_y = top * rawHeight;
            unsigned int crop_w = (right - left) * rawWidth;
            unsigned int crop_h = (bottom - top) * rawHeight;

            if (boxes[i].score >= threshold/100.0 && boxes[i].label != 0) {
                syslog(LOG_INFO, "Object %d: Classes: %s - Scores: %f - Locations: [%f,%f,%f,%f]",
                    i, labels[boxes[i].label-1], boxes[i].score, top, left, bottom, right);

                unsigned char* crop_buffer = crop_interleaved(cropAddr, rawWidth, rawHeight, CHANNELS,
                                                                crop_x, crop_y, crop_w, crop_h);

                unsigned long jpeg_size = 0;
                unsigned char* jpeg_buffer = NULL;
                struct jpeg_compress_struct jpeg_conf;
                set_jpeg_configuration(crop_w, crop_h, CHANNELS, quality, &jpeg_conf);
                buffer_to_jpeg(crop_buffer, &jpeg_conf, &jpeg_size, &jpeg_buffer);
                char file_name[32];
                snprintf(file_name, sizeof(char) * 32, "/tmp/detection_%i.jpg", i);
                jpeg_to_file(file_name, jpeg_buffer, jpeg_size);
                free(crop_buffer);
                free(jpeg_buffer);
            }

        }

        // Release frame reference to provider.
        returnFrame(provider, buf);
        returnFrame(provider_raw, buf_hq);
    }

    syslog(LOG_INFO, "Stop streaming video from VDO");
    if (!stopFrameFetch(provider)) {
        goto end;
    }

    ret = true;

end:
    if (provider) {
        destroyImgProvider(provider);
    }
    if (provider_raw) {
        destroyImgProvider(provider_raw);
    }
    // Only the model handle is released here. We count on larod service to
    // release the privately loaded model when the session is disconnected in
    // larodDisconnect().
    larodDestroyMap(&ppMap);
    larodDestroyMap(&cropMap);
    larodDestroyModel(&ppModel);
    larodDestroyModel(&model);
    if (conn) {
        larodDisconnect(&conn, NULL);
    }
    if (larodModelFd >= 0) {
        close(larodModelFd);
    }
    if (larodInputAddr != MAP_FAILED) {
        munmap(larodInputAddr, inputWidth * inputHeight * CHANNELS);
    }
    if (larodInputFd >= 0) {
        close(larodInputFd);
    }
    if (ppInputAddr != MAP_FAILED) {
        munmap(ppInputAddr, inputWidth * inputHeight * CHANNELS);
    }
    if (ppInputFd >= 0) {
        close(ppInputFd);
    }
    if (cropAddr != MAP_FAILED) {
        munmap(cropAddr, rawWidth * rawHeight * CHANNELS);
    }
    if (cropFd >= 0) {
        close(cropFd);
    }
    if (larodOutput1Addr != MAP_FAILED) {
        munmap(larodOutput1Addr, TENSOR1SIZE);
    }

    if (larodOutput2Addr != MAP_FAILED) {
        munmap(larodOutput2Addr, TENSOR2SIZE);
    }

    if (larodOutput1Fd >= 0) {
        close(larodOutput1Fd);
    }

    if (larodOutput2Fd >= 0) {
        close(larodOutput2Fd);
    }


    larodDestroyJobRequest(&ppReq);
    larodDestroyJobRequest(&infReq);
    larodDestroyTensors(conn, &inputTensors, numInputs, &error);
    larodDestroyTensors(conn, &outputTensors, numOutputs, &error);
    larodClearError(&error);

    if (labels) {
        freeLabels(labels, labelFileData);
    }
    if (boxes){
        free(boxes);
    }
    if (rgb_image){
        free(rgb_image);
    }

    syslog(LOG_INFO, "Exit %s", argv[0]);
    return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}
