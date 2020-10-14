# sm64ex
Fork dari [sm64-port/sm64-port](https://github.com/sm64-port/sm64-port) dengan ciri tambahan.

Jangan ragu-ragu untuk melaporkan pepijat dan memberi sumbangan, tetapi ingat, tidak boleh **memuat naik aset berhak cipta**. 
Jalankan `./extract_assets.py --clean && make clean` atau `make distclean` untuk membuang kandungan yang berasal dari ROM.

Sila sumbangkan **dahulu** ke [nightly branch](https://github.com/sm64pc/sm64ex/tree/nightly/). Fungsi baru akan digabungkan ke master setelah siap diuji.

*Baca ini dalam bahasa lain: [Español](README_es_ES.md), [Português](README_pt_BR.md), [简体中文](README_zh_CN.md) atau [Bahasa Melayu](README_ms_MY.md).*

## Ciri-ciri baru

 * Menu pilihan dengan pelbagai tetapan, termasuk pemetaan semula butang.
 * Pilihan untuk pemuatan data luaran (setakat ini hanya tekstur dan papan suara yang dipasang), memberikan sokongan untuk pek tekstur tersuai.
 * Pilihan untuk rupa kamera analog dan tetikus (menggunakan [Puppycam](https://github.com/FazanaJ/puppycam)).
 * Pilihan untuk perender berasaskan OpenGL1.3 untuk mesin yang lebih tua, serta perender asli GL2.1, D3D11 dan D3D12 dari Emill's [n64-fast3d-engine](https://github.com/Emill/n64-fast3d-engine/).
 * Pilihan untuk menyahaktifkan jarak lukisan.
 * Pilihan untuk pembaikan model dan tekstur (mis. Tekstur asap).
 * Langkau cutscenes pengenalan Peach & Lakitu dengan pilihan CLI `--skip-intro`.
 * Menu cheats di Options (aktifkan dengan `--cheats` atau dengan menekan L tiga kali di menu pause).
 * Sokongan untuk kedua-dua fail simpanan little-endian dan big-endian (bermaksud anda boleh menggunakan fail simpan dari sm64-port dan kebanyakan emulator), serta format simpanan berasaskan teks pilihan.

Perubahan terbaru di Nightly telah memindahkan laluan fail simpan dan konfigurasi ke `%HOMEPATH%\AppData\Roaming\sm64pc` pada Windows dan `$HOME/.local/share/sm64pc` di Linux. Tingkah laku ini dapat diubah dengan `--savepath` pilihan CLI.
For example `--savepath .` akan membaca simpanan dari direktori semasa (yang tidak selalu sepadan dengan direktori exe, tetapi selalunya ia berlaku);
   `--savepath '!'` akan membaca simpanan dari direktori executable.

## Pembangunan
Untuk arahan pembangunan, sila rujuk di [wiki](https://github.com/sm64pc/sm64ex/wiki).

**Pastikan anda mempunyai MXE terlebih dahulu sebelum cuba compile untuk Windows di Linux dan WSL. Ikuti panduan di wiki.**
