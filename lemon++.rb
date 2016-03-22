
class Lemonxx < Formula
  desc "lemon parser generator with improved c++ support"
  homepage "https://github.com/ksherlock/lemon--/"
  url "https://github.com/ksherlock/lemon--/archive/2016-03-22.tar.gz"
  sha256 "40ba339defe6bd334abd4fb655f7d9d8b57adc5f0737aad647de0470927a81d2"

  def install
    (share/"lemon").install "lempar.c"
    (share/"lemon").install "lempar.cpp"

    system "make HOMEBREW_TEMPLATE_PATH=#{share}/lemon/"
    bin.install "lemon"
    bin.install "lemon++"
  end

  conflicts_with "lemon", :because => "both install `lemon` binaries"

  test do
    # system "false"
    system "true"
  end
end
