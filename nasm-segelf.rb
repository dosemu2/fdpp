class NasmSegelf < Formula
  desc "nasm-segelf"
  homepage "https://github.com/stsp/nasm"
  url "https://github.com/stsp/nasm.git", branch: "elf16"
  version "2.17"
  sha256 ""
  license "BSD-2-Clause"

  depends_on "autoconf" => :build
  depends_on "automake" => :build
  depends_on "gcc" => :build
  depends_on "perl" => :build
  depends_on "make" => :build

  def install
    system "./autogen.sh"
    system "./configure", "--prefix=#{prefix}"
    system "make", "install"
  end

  test do
    (testpath/"foo.s").write <<~EOS
      mov eax, 0
      mov ebx, 0
      int 0x80
    EOS

    system "#{bin}/nasm-segelf", "foo.s"
    code = File.open("foo", "rb") { |f| f.read.unpack("C*") }
    expected = [0x66, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x66, 0xbb,
                0x00, 0x00, 0x00, 0x00, 0xcd, 0x80]
    assert_equal expected, code
  end
end
