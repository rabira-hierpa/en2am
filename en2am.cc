/*
 * Copyright (c) 2015, Misgana Bayetta <misgana.bayetta@gmail.com>
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3. All advertising materials mentioning features or use of this software
 must display the following acknowledgement:
 This product includes software developed by the <organization>.
 4. Neither the name of the <organization> nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY Misgana Bayetta ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

#include "en2am.h"

using namespace std;

class translator {
private:
    string sconverted;
    map<string, string>& conv_tbl;

    vector<string> match_key(const string& word);
    vector<string> tokenize(string text, char sep);

public:
    translator(map<string, string>& _conv_tbl) :
            conv_tbl(_conv_tbl)
    {
    }
    virtual ~translator()
    {
    }

    string to_amharic(const string& english_str);
};

vector<string> translator::tokenize(string text, char sep)
{
    vector<string> tokens;
    int start = 0, end = 0;

    while ((end = text.find(sep, start)) != string::npos) {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(text.substr(start));

    return tokens;
}

vector<string> translator::match_key(const string& word)
{
    string matched_key = "";

    for (auto iter = conv_tbl.begin(); iter != conv_tbl.end(); ++iter) {        
        string key = iter->first;

        if (key.size() > word.size())
            continue;
        
        bool match = true;
        for (int i = 0; i < key.size(); i++) {
            if (key.at(i) != word.at(i)) {
                match = false;
                break;
            }            
        }

        if (match) {
            if (matched_key == "") {
                matched_key = key;
            } else {
                if (key.size() > matched_key.size())
                    matched_key = key;
            }
        }
    }

    if (matched_key != "")
        return vector<string> { matched_key, conv_tbl[matched_key] }; //no match found
    else
        return vector<string> { };
}

string translator::to_amharic(const string& english_str)
{
    vector<string> lines = tokenize(english_str, '\n');
    bool scape = false;
    for (auto lniter = lines.begin(); lniter != lines.end(); ++lniter) {
        string line = *lniter;

        vector<string> winp = tokenize(line, ' ');

        for (size_t i = 0; i < winp.size(); i++) {
            string wout; //Amharic output. A unicode string
            string word = winp[i];
            while (word.size() != 0) {

                vector<string> matched = match_key(word); // {en_key,am_equivalent}
                if (matched.size() != 2) {
                    wout += word;
                    break;
                }
                wout += matched[1]; //append translation token.
                //trim from the front
                word.erase(0, matched[0].size());
            }

            sconverted += wout;
            if (i != winp.size())
                sconverted += " ";
        }

        if (lniter != lines.end() - 1)
            sconverted += "\n";
    }

    return sconverted;
}

string readf(const string& path)
{
    try {
        ifstream in(path);
        stringstream buffer;
        buffer << in.rdbuf();
        string input = buffer.str();
        in.close();
        return input;
    } catch (ifstream::failure e) {
        cout << "ERROR \n" << e.what() << endl;
    }
}

void writef(string& input, const string& fname)
{
    ofstream outf;
    outf.open(fname.c_str());
    outf << input;
    outf.close();
}

int main(int argc, char** args)
{
    const string usage =
            "Usage \n  en2am [input_file_path] [output_file_path] \n  en2am [\"STRING\"] \n";
    if (argc < 2 or argc > 3) {
        std::cout << "Wrong number of arguments.\n " << usage << std::endl;
        return 0;
    }

    string raw_input;
    string out_path;

    if (argc == 3) {
        string in_path(args[1]);
        raw_input = readf(string(in_path));
        out_path = (string) args[2];
    } else {
        raw_input = (string) args[1];
    }

    translator ts(KEY_MAP);
    string converted = ts.to_amharic(raw_input);

    if (out_path.size() != 0)
        writef(converted, out_path);
    else
        cout << converted << endl;

    return 0;
}
