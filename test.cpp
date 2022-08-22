#include <bits/stdc++.h>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
using namespace std;


int main()
{
    fs::path p = fs::current_path();
    
    string path;
    std::cout << "The current path " << p << " decomposes into:\n"
              << "root-path " << p.root_path() << '\n'
              << "relative path " << p.relative_path() << '\n';
    cin >> path;

    p.append(path);
    cout << "New path: " << p << endl;
    string cmd = "cd " + p.generic_u8string();
    // system(cmd.c_str());
    FILE *fpipe;
    char c = 0;

    if (0 == (fpipe = (FILE*)popen(cmd.c_str(), "r"))) {
        perror("Não foi possível executar o comando");
    }

    while (fread(&c, sizeof c, 1, fpipe)) {
        printf("%c", c);
    }

    int code = pclose(fpipe);
    cout << code << endl;
    cout << "Fim" << endl;
}