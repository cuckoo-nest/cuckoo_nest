#include "UnixSerialPort.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <sys/ioctl.h>
#include <cerrno>
#include <spdlog/spdlog.h>

static speed_t BaudRateToSpeed(BaudRate b)
{
    switch (b)
    {
        case BaudRate::Baud9600: return B9600;
        case BaudRate::Baud19200: return B19200;
        case BaudRate::Baud38400: return B38400;
        case BaudRate::Baud57600: return B57600;
        case BaudRate::Baud115200: return B115200;
        default: return B115200;
    }
}

UnixSerialPort::UnixSerialPort(const std::string &port)
    : ISerialPort(port), portName(port), fd(-1)
{
}

UnixSerialPort::~UnixSerialPort()
{
    Close();
}

bool UnixSerialPort::Open(BaudRate baudRate)
{
    fd = ::open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        spdlog::error("UnixSerialPort: open failed: {}", std::strerror(errno));
        return false;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0)
    {
        spdlog::error("UnixSerialPort: tcgetattr failed: {}", std::strerror(errno));
        ::close(fd);
        fd = -1;
        return false;
    }

    speed_t speed = BaudRateToSpeed(baudRate);
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0;

    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD | CSTOPB | CRTSCTS);
    tty.c_lflag &= ~(ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL | IEXTEN);
    tty.c_oflag &= ~OPOST;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        spdlog::error("UnixSerialPort: tcsetattr failed: {}", std::strerror(errno));
        ::close(fd);
        fd = -1;
        return false;
    }

    return true;
}

void UnixSerialPort::Close()
{
    if (fd >= 0)
    {
        ::close(fd);
        fd = -1;
    }
}

int UnixSerialPort::Read(char* buffer, int bufferSize)
{
    if (fd < 0) return 0;
    ssize_t r = ::read(fd, buffer, static_cast<size_t>(bufferSize));
    if (r < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        spdlog::error("UnixSerialPort: read failed: {}", std::strerror(errno));
        return 0;
    }
    return static_cast<int>(r);
}

int UnixSerialPort::Write(const std::vector<uint8_t> &data)
{
    if (fd < 0) return -1;
    ssize_t w = ::write(fd, data.data(), data.size());
    if (w < 0)
    {
        spdlog::error("UnixSerialPort: write failed: {}", std::strerror(errno));
        return -1;
    }
    return static_cast<int>(w);
}

int UnixSerialPort::SendBreak(int durationMs)
{
    if (fd < 0) return -1;
    int rc = tcsendbreak(fd, 0);
    if (rc != 0)
    {
        spdlog::error("UnixSerialPort: tcsendbreak failed: {}", std::strerror(errno));
        return -1;
    }
    return 1;
}

int UnixSerialPort::Flush()
{
    if (fd < 0) return -1;
    if (tcflush(fd, TCIOFLUSH) != 0)
    {
        spdlog::error("UnixSerialPort: tcflush failed: {}", std::strerror(errno));
        return -1;
    }
    return 1;
}
