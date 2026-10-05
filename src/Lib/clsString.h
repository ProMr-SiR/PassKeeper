#pragma once

#include <string>
#include <vector>

using namespace std;

/**
 * @class clsString
 * 
 * 
 */
class clsString
{
public:
    /**
     * 
     * 
     * 
     *   Input:  "1#//#Google#//#test@gmail.com#//#MyPass"
     *   Delim:  "#//#"
     *   Output: ["1", "Google", "test@gmail.com", "MyPass"]
     * 
     * 
     * 
     * 
     * 
     * @param Delim الفاصل (يمكن أن يكون أكثر من حرف مثل "#// 
     * 
     */
    static vector<string> Split(string S1, string Delim)
    {
        vector<string> vString;
        int pos = 0;
        string sWord;

        // 
        while ((pos = S1.find(Delim)) != string::npos)
        {
            sWord = S1.substr(0, pos);  // 
            if (sWord != "")
                vString.push_back(sWord); // 

            S1.erase(0, pos + Delim.length()); // 
        }

        // 
        if (S1 != "")
            vString.push_back(S1);

        return vString;
    }

    /**
     * 
     * 
     * 
     *   Input:  "google"
     *   Output: "GOOGLE"
     * 
     * 
     * 
     * 
     * 
     * 
     */
    static string UpperAllString(string S1)
    {
        for (int i = 0; i < S1.length(); i++)
        {
            S1[i] = toupper(S1[i]); // 
        }
        return S1;
    }
};
