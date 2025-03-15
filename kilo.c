/*
1. 首先你得先有一个主循环
2. 在开始时，使终端从规范模式(canonical mode/cooked mode)进入到原始模式(raw mode)
    a.turn off echo
    b.turn off canonical Mode
    c.turn off Ctrl-C and Ctrl-Z signals
    d.Disable Ctrl-S and Ctrl-Q
    e.Disable Ctrl-V
    f.Fix Ctrl-M
    g.turn off all output processing
    h.miscellaneous flags
3. 在结束时，退出 raw mode
*/

/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void die(const char *s) {
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("tcsetattr");
    }
}

void enableRawMode() {
    // 读取终端属性
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        die("tcgetattr");
    }
    // 在程序退出时，自动调用diableRawMode()
    atexit(disableRawMode);

    // termios是终端属性结构体
    struct termios raw = orig_termios;
    // 禁用终端某些功能，使其进入 raw mode
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON | IGNCR);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    // 将raw应用到新的终端的属性上
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("tcsetattr");
    }
}

/*** init ***/

int main() {
    enableRawMode();

    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
            die("read");
        }

        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') {
            break;
        }
    }
    
    return 0;
}