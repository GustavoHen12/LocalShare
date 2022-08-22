#ifndef __COMMANDS__
#define __COMMANDS__

#include <bits/stdc++.h>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>

namespace fs = std::filesystem;
using namespace std;

#include"protocol.h"

#define CMD_CD      1
#define CMD_LS      2
#define CMD_MKDIR   3
#define CMD_GET     4
#define CMD_PUT     5

#define MAX_ATEMPTS 10

int getCommandCode(string command);

/*********** COMANDO DO CLIENTE ***********/
void cd_client(string directory, int attempts=0);

/*********** COMANDO DO SERVIDOR ***********/
void cd_server(string& directory, fs::path& current_path);

#endif

