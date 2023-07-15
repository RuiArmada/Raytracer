{ pkgs, ... }:

{
  # https://devenv.sh/packages/
  packages = with pkgs; [
    pkg-config
    openimageio.dev
    opencolorio
    cmake
    fmt
    zlib
  ];

  env.ZLIB_LIBRARY = "${pkgs.zlib}/lib";
  env.ZLIB_INCLUDE_DIR = "${pkgs.zlib.dev}/include";
}
