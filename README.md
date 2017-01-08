# 3ds-crypto-server
ARM11 Userland homebrew for 3ds crypto over the network.

Requires my [crypto-patches](https://github.com/SciresM/Luma3DS/tree/crypto-patches) branch of [Luma3DS](https://github.com/AuroraWright/Luma3DS).

Also requires very recent ctrulib (for PS_EncryptDecryptAES to actually work).

Probably also requires you to enable "Patch SVC/service access" in Luma config.

Gets about ~925 KB/s on my console/network.

See crypto_client.py for an example client.

![Example of NCCH decryption](/img/preview.jpg)