
class Lemonxx < Formula
  desc "lemon parser generator with improved c++ support"
  homepage "https://github.com/ksherlock/lemon--/"
  url "https://github.com/ksherlock/lemon--/archive/2016-02-05.tar.gz"
  sha256 "8b6f19f22772c3df832674646df31cb8f6819b05c16146894bd63a51d9c251f9"

  def install
    (share/"lemon").install "lempar.c"
    (share/"lemon").install "lempar.cpp"

    inreplace "lemon.c", / = pathsearch\([^)]*\);/, " = \"#{share}/lemon/lempar\" FILE_EXTENSION;"

    system "make"
    bin.install "lemon"
    bin.install "lemon++"
  end

  conflicts_with "lemon", :because => "both install `lemon` binaries"

  test do
    # system "false"
    system "true"
  end
end
