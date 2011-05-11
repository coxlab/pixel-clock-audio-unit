#!/bin/bash
protoc --proto_path=../ --python_out=./ ../pixel_clock_info.proto
