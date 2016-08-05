
class Lemonxx < Formula
  desc "lemon parser generator with improved c++ support"
  homepage "https://github.com/ksherlock/lemon--/"
  url "https://github.com/ksherlock/lemon--/archive/2016-08-05.tar.gz"
  sha256 "a00c748fc368bf708c300f204e110948c7937168a11cdfd0fabf33b8aee75c07"

  def install
    (share/"lemon").install "lempar.c"
    (share/"lemon").install "lempar.cpp"
    (share/"lemon").install "lempar.cxx"
    (include).install "lemon_base.h"

    system "make HOMEBREW_TEMPLATE_PATH=#{share}/lemon/"
    bin.install "lemon"
    bin.install "lemon++"
    bin.install "lemon--"
  end

  conflicts_with "lemon", :because => "both install `lemon` binaries"

  test do
    # system "false"
    system "true"
  end
end
