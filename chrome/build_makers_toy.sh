rm -rf build_ninjaPCR;
cp -r NinjaPCR  build_MakersToyPCR;

# Replace "NinjaPCR" with "Makers Toy PCR"
find ./build_MakersToyPCR/ -type f | xargs sed -i "" 's/NinjaPCR/Makers Toy PCR/g'

zip -r MakersToyPCR.zip build_MakersToyPCR

open https://chrome.google.com/webstore/developer/dashboard
