# Maintainer: Byron Mallett <mystfit.code@gmail.com>
pkgname=masquerade-ball-git
pkgver=0.1.0.r8.g1a87044
pkgrel=1
pkgdesc="Terminal-based side-scrolling physics platformer in C++20, inspired by Vib-Ribbon"
arch=('x86_64')
url="https://github.com/Mystfit/MasqueradeBall"
license=('MIT')
depends=('gcc-libs')
makedepends=('cmake' 'git' 'gcc')
provides=('masquerade-ball')
conflicts=('masquerade-ball')
source=("${pkgname}::git+${url}.git")
sha256sums=('SKIP')

pkgver() {
    cd "${pkgname}"
    git describe --long --tags 2>/dev/null | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g' ||
    printf "0.1.0.r%s.g%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
    cmake -B build -S "${pkgname}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DMASQUERADE_ASSETS_DIR=/usr/share/masquerade-ball \
        -Wno-dev
    cmake --build build -j"$(nproc)"
}

package() {
    DESTDIR="${pkgdir}" cmake --install build
    install -Dm644 "${pkgname}/LICENSE" \
        "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
}
