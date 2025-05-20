#include <curses.h>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <sstream>

std::string formatLengthMS(long long MS) {
    long long seconds = MS / 1000000;
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << secs;
    return oss.str();
}

std::string formatLenghtM(int st) {
    int m = st / 60;
    int s = st % 60;

    std::ostringstream oss;
    oss << m << ":" << std::setw(2) << std::setfill('0') << s;
    return oss.str();
}

std::string exec_command(const std::string& cmd) {
    char buffer[128];
    FILE *fp;
    std::string result;

    fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        perror("ERROR AT EXECUTING COMMAND, DO YOU HAVE PLAYERCTL INSTALLED?");
        return "";
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        result += buffer;
    }

    fclose(fp);

    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }

    return result;
}

long long safe_stoll(const std::string& str) {
    try {
        return std::stoll(str);
    } catch (...) {
        return 0;
    }
}

int main() {
    initscr();
    curs_set(0);
    noecho();
    cbreak();

    int width = COLS / 1.1;
    int height = LINES / 1.1;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;

    WINDOW *win = newwin(height, width, starty, startx);

    nodelay(win, TRUE);
    keypad(win, TRUE);

    bool run = true;
    int ch;

    while (run) {
        ch = wgetch(win);
        werase(win);
        if (ch != ERR) {
            if (ch == 'q') {
                run = false;
                break;
            }
            if (ch == KEY_F(1)) {
                exec_command("playerctl -a play-pause");
                erase();
                wrefresh(win);
            }
            if (ch == KEY_F(2)) {
                exec_command("playerctl -a next");
                erase();
                wrefresh(win);
            }
            if (ch == KEY_F(3)) {
                exec_command("playerctl -a previous");
                erase();
                wrefresh(win);
            }
            if (ch == KEY_F(4)) {
                exec_command("playerctl -a volume 0");
                erase();
                wrefresh(win);
            }
            if (ch == KEY_F(5)) {
                exec_command("playerctl -a volume +0.1");
                erase();
                wrefresh(win);
            }
            if (ch == KEY_F(6)) {
                exec_command("playerctl -a volume -0.1");
                erase();
                wrefresh(win);
            }
        }
		
        std::string title = exec_command("playerctl -a metadata title");
        std::string artist = exec_command("playerctl -a metadata artist");
        std::string duration = exec_command("playerctl -a metadata mpris:length");
        std::string position = exec_command("playerctl -a position");
        std::string status = exec_command("playerctl -a status");

        if(title.find("not") != std::string::npos)
        {}

        long long dur = safe_stoll(duration);
        long long pos = safe_stoll(position);

        std::string finalPosDur;
        if (dur == 0 || pos == 0) {
            finalPosDur = "Loading...";
        } else {
            std::string formatted_duration = formatLengthMS(dur);
            std::string formatted_position = formatLenghtM(pos);
            finalPosDur = formatted_duration + " / " + formatted_position;
        }

        mvwprintw(win, 2, (width - title.length()) / 2, "%s", title.c_str());
        mvwprintw(win, 3, (width - status.length()) / 2, "%s", status.c_str());
        mvwprintw(win, 4, (width - 40) / 2, "---------------------------------------");
        mvwprintw(win, 5, (width - artist.length()) / 2, "%s", artist.c_str());
        mvwprintw(win, 6, (width - 40) / 2, "---------------------------------------");
        mvwprintw(win, 7, (width - finalPosDur.length()) / 2, "%s", finalPosDur.c_str());

        mvwprintw(win, 13, (width - 50) / 2, "F1: Pause/Play | F2: Next | F3: Previous | q: Quit");
        mvwprintw(win, 14, (width - 42) / 2, "F4: Mute | F5: Volume Up | F6: Volume Down");

        box(win, 0, 0);
        wrefresh(win);
        refresh();
        napms(100);
    }

    delwin(win);
    endwin();
    return 0;
}
