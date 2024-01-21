// Meter Tools / Darius Kellermann <kellermann@protonmail.com>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <iostream>

class Errno_ex : public std::exception {
  public:
    Errno_ex() : errno_{errno} {}

    const char *what() const noexcept override { return std::strerror(errno_); }

  private:
    int errno_;
};

namespace {

  auto get_serial_port_config(const int serial_port_fd) {
    auto config = termios{};
    if (tcgetattr(serial_port_fd, &config) != 0) {
      throw Errno_ex{};
    }
    return config;
  }

  void save_serial_port_config(const int serial_port_fd,
                               const termios &config) {
    if (tcsetattr(serial_port_fd, TCSANOW, &config) != 0) {
      throw Errno_ex{};
    }
  }

} // namespace

class Serial_port {
  public:
    explicit Serial_port(const char *device) : fd_{open(device, O_RDWR)} {
      if (fd_ > 0) {
        auto config = get_serial_port_config(fd_);
        config.c_cflag &= ~PARENB;                        // no parity
        config.c_cflag &= ~CSTOPB;                        // one stop bit
        config.c_cflag = (config.c_cflag & ~CSIZE) | CS8; // 8 data bits
        config.c_cflag &= ~CRTSCTS;                       // no flow control
        config.c_cflag |= CREAD | CLOCAL; // read & ignore control lines
        config.c_lflag |= ICANON; // canonical mode is helpful for our purposes
        config.c_iflag &= ~(IXON | IXOFF
                            | IXANY); // disable software flow control
        config.c_cc[VMIN] = 1;
        config.c_cc[VTIME] = 0;
        cfsetispeed(&config, B9600);
        cfsetospeed(&config, B9600);
        save_serial_port_config(fd_, config);
      } else {
        throw Errno_ex{};
      }
    }

    std::string read_line() {
      if (read(fd_, data(rx_buffer_), size(rx_buffer_)) > 0) {
        return data(rx_buffer_);
      }
      throw Errno_ex{};
    }

  private:
    int fd_;
    std::array<char, 80> rx_buffer_{};
};

int main(const int argc, const char *const *const argv) try {
  if (argc < 2) {
    std::cout << "usage: statistics <serial port>" << std::endl;
    return 0;
  }
  auto serial = Serial_port{argv[1]};
  while (true) {
    std::cout << serial.read_line();
  }
} catch (const std::exception &ex) {
  std::cerr << "error: " << ex.what() << std::endl;
  return -1;
}
