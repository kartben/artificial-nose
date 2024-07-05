# Contributing

Contributions to ESP-NN project in the form of pull requests, bug reports, and feature requests are welcome!

This document covers various topics related to contributions to the ESP-NN projects. Please read it if you plan to submit a PR!

## CLA

We require accepting the contributor's license agreement for all pull requests. When opening a pull request the first time you will be prompted to sign the CLA by the [CLA Assistant](https://cla-assistant.io/) service.

## Large-scale Changes

If you'd like to propose a change to the existing APIs or a large-scale refactoring of the implementation, we recommend opening an issue first to discuss this.

## Updating the Benchmarks Table

The benchmarks table in [README.md](README.md) contains benchmarks for ESP32-S3. The benchmarks are collected by running the app in [test_app](test_app/) directory. Please update this table if you have changed the implementations of some of the functions or added the new ones.

## Releasing a new version

Maintainers should follow the steps below to release a new version of ESP-NN component. Assuming the new version is `vX.Y.Z`:

1. Ensure you are on the latest `master` branch:
   ```bash
   git checkout master
   git pull --ff-only origin master
   ```
1. Create the new tag:
   ```bash
   git tag -s -a -m "vX.Y.Z" vX.Y.Z
   ```
1. Push the tag and the branch to the internal repository:
   ```bash
   git push origin vX.Y.Z
   ```
1. CI will automatically push the tag to Github and will upload the new version to the IDF Component Registry.
1. Go to https://github.com/espressif/esp-nn/releases and create a release from the tag vX.Y.Z.
1. Write the release notes and publish the release.
