# Comparison Notes

This project was compared at a high level against the better-known
ESP32 Marauder ecosystem, especially the official
[`justcallmekoko/ESP32Marauder`](https://github.com/justcallmekoko/ESP32Marauder)
repository and its documented hardware variants.

## Patterns Common In Those Builds

- multiple hardware targets
- persistent device settings
- battery status integration
- onboard storage for captured or exported data
- a device-oriented operator UI

## Safe Changes Adopted Here

To make this repo more complete without inheriting offensive wireless features,
the following patterns were added:

- persisted settings in NVS
- SPIFFS-backed scan logs
- richer serial operator commands
- clearer hardware extension points
- CI-based firmware builds on GitHub Actions

## Not Adopted

This repo intentionally does not add:

- deauthentication
- beacon spam
- credential capture
- packet injection workflows
- evil portal features

The goal is a solid handheld scanner foundation, not a clone of the offensive
capabilities found in some Marauder distributions.
