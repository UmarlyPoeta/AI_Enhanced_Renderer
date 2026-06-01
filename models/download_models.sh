#!/usr/bin/env bash
# Download super-resolution models for OpenCV dnn_superres.
# Run from the models/ directory:  bash download_models.sh
set -euo pipefail

cd "$(dirname "$0")"

echo "Downloading ESPCN..."
wget -nc https://raw.githubusercontent.com/fannymonori/TF-ESPCN/master/export/ESPCN_x2.pb
wget -nc https://raw.githubusercontent.com/fannymonori/TF-ESPCN/master/export/ESPCN_x3.pb
wget -nc https://raw.githubusercontent.com/fannymonori/TF-ESPCN/master/export/ESPCN_x4.pb

echo "Downloading FSRCNN..."
wget -nc https://github.com/Saafke/FSRCNN_Tensorflow/raw/master/models/FSRCNN_x2.pb
wget -nc https://github.com/Saafke/FSRCNN_Tensorflow/raw/master/models/FSRCNN_x3.pb
wget -nc https://github.com/Saafke/FSRCNN_Tensorflow/raw/master/models/FSRCNN_x4.pb

echo "Downloading EDSR (optional, slower but higher quality)..."
wget -nc https://github.com/Saafke/EDSR_Tensorflow/raw/master/models/EDSR_x2.pb || true
wget -nc https://github.com/Saafke/EDSR_Tensorflow/raw/master/models/EDSR_x3.pb || true
wget -nc https://github.com/Saafke/EDSR_Tensorflow/raw/master/models/EDSR_x4.pb || true

echo "Done. Available: ESPCN + FSRCNN (x2/x3/x4). EDSR if download succeeded."
