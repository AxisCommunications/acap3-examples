# Frequently asked questions

Below are some of the frequently asked questions.

## What is Larod?

Larod is a machine learning service that typically runs on target. By using the lib it's shipped with (liblarod) you can communicate with the service and use its features. For now this is limited to uploading neural network models (converted into the proper format) and running inferences on them. In the future larod might also have features which aid users in preprocessing (e.g. image format conversion and scaling) input data to a neural network.

## Which products can make use of deep learning with larod?

Certainly products with dedicated hardware (e.g. the Edge TPU), but actually also any product with an ARM CPU (using Tensorflow Lite) - so most products. The performance is of course worse than if you would have used dedicated hardware but it's actually enough for many use cases.

## Why does larod exist?

Axis already has several HW platforms (e.g. the Edge TPU and ARM CPU) providing neural network acceleration. The software through which one communicates with these for simple tasks such as running inference differ. So larod's main task is to provide a layer between the HW specific software to provide app developers with a unified API which can be used on all platforms, but a layer with very little overhead. Another important task is to arbitrate between different processes (apps) requesting access to the same hardware. Some HW's software (like the Edge TPU lib) only allows for one process to use the HW, but making larod that one process allows for several processes to use the hardware fairly through larod.

## How can I set-up network proxy settings on the Axis device?

You may need to run the following script on the Axis device depending upon your network requirements.

```bash
ssh root@<axis_device_ip>
```

Run on the device:

```bash
#!/bin/sh

# Setup proxy for dockerd

  cat >> /etc/systemd/system/sdkrun_dockerd.service <<EOF
  [Service]
  Environment="HTTP_PROXY=http://<myproxy.com>:<port>"
  Environment="HTTPS_PROXY=http://<myproxy>:<port>"
  Environment="NO_PROXY=localhost,127.0.0.0/8,10.0.0.0/8,192.168.0.0/16,172.16.0.0/12,.<domain>"
  EOF

  systemctl daemon-reload
  systemctl restart sdkrun_dockerd

  exit
```

## How to enable SSH on an Axis device

You can enable SSH on an Axis device either through the device's web interface or by calling a VAPIX API from command-line.

### Enable SSH from device web interface

#### Old web interface:

1. Go to the following URL where <AXIS_DEVICE_IP> is the IP number of your Axis device:
    - Axis OS < 10.6 `http://<AXIS_DEVICE_IP>/#settings/system/tools/plainconfig`
    - Axis OS >= 10.6 `http://<AXIS_DEVICE_IP>/aca/index.html#settings/system/tools/plainconfig`
2. Click on `Network` in the list.
3. Under `SSH` select `Enabled`.
4. Scroll to the bottom of the page and click button `Save`.

#### New web interface:

1. Go to `http://<AXIS_DEVICE_IP>/camera/index.html#/system/plainConfig` where <AXIS_DEVICE_IP> is
the IP number of your Axis device.
2. Select the `Network` group from the drop-down menu.
3. Under `Network / SSH` select `Enabled`.
4. Scroll to the bottom of the page and click button `Save`.

### Enable SSH from command-line

This is examplified using curl:

```sh
curl -u <USER>:<PASS> "http://<AXIS_DEVICE_IP>/axis-cgi/admin/param.cgi?action=update&Network.SSH.Enabled=yes"
```

> More options may be required depending on your network setup.
