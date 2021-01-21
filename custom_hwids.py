Import("env")

board_config = env.BoardConfig()
# should be array of VID:PID pairs
board_config.update("build.hwids", [
  ["0x27dc", "0x16c2"],  # 1st pair
  ["0x27dc", "0x00C2"]  # 2nd pair, etc.
])
board_config.update("build.usb_product", "Button Box")
board_config.update("name", "Pro Micro Button Box")
board_config.update("url", "https://smithem.com")
board_config.update("vendor", "BillSmithem")