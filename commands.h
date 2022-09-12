#ifndef __COMMANDS__
#define __COMMANDS__

#include <bits/stdc++.h>
#include <experimental/filesystem>
#include <sys/types.h>
#include <sys/stat.h>

namespace fs = std::experimental::filesystem;
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
void cd_client(string directory, fs::path& current_path, fs::path& server_path, bool server_cmd=true);

void ls_client(string parameter);

void put_client(string parameter, fs::path& current_path);

void get_client(string parameter, fs::path& current_path);

void mkdir_client(string parameter, fs::path& current_path);

/*********** COMANDO DO SERVIDOR ***********/
void cd_server(string& directory, fs::path& current_path);

void ls_server(string parameter, fs::path& current_path);

void put_server(string parameter, fs::path& current_path);

void get_server(string parameter, fs::path& current_path);

void mkdir_server(string parameter, fs::path& current_path);

#endif

