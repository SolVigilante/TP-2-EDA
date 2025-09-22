/**
 * @brief Lequel? language identification based on trigrams
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2022-2023
 *
 * @cite https://towardsdatascience.com/understanding-cosine-similarity-and-its-application-fd42f585296a
 */

#include <cmath>
#include <codecvt>
#include <locale>
#include <iostream>

#include "Lequel.h"

using namespace std;

/**
 * @brief Builds a trigram profile from a given text.
 *
 * @param text Vector of lines (Text)
 * @return TrigramProfile The trigram profile
 */
TrigramProfile buildTrigramProfile(const Text &text)
{
    wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    TrigramProfile TrigramProfile;
    string trigram;
    wstring unicodeString, unicodeTrigram;

    int cantTrigrams = 0;

 for (auto line : text)
    {
        if ((line.length() > 0) && (line[line.length() - 1] == '\r'))
                line = line.substr(0, line.length() - 1);
        
        unicodeString = converter.from_bytes(line);

        if (unicodeString.length() < 3)
                continue;

        for(int i=0; i < unicodeString.length() - 2; i++)
        {
            unicodeTrigram=unicodeString.substr(i,3); // Extracts a trigram
            trigram = converter.to_bytes(unicodeTrigram);// Converts it back to UTF-8 string

            if(TrigramProfile.find(trigram) != TrigramProfile.end())
            {
                TrigramProfile[trigram] += 1;
            } else 
            {
                TrigramProfile[trigram] = 1;
            }

            cantTrigrams++;
        }
        
        if(cantTrigrams >= 600){
            break; // Limits to first 600 trigrams
        }
    }

    return TrigramProfile;
}

/**
 * @brief Normalizes a trigram profile.
 *
 * @param trigramProfile The trigram profile.
 */
void normalizeTrigramProfile(TrigramProfile &trigramProfile)
{
    double sumFrequencies = 0;

    for ( auto &triagramList : trigramProfile)
    {
        sumFrequencies += (double)triagramList.second * (double)triagramList.second;
        
    }

    sumFrequencies = sqrt(sumFrequencies);

    for ( auto &triagramList : trigramProfile)
    {
        triagramList.second = (float)((double)triagramList.second / sumFrequencies);
    }
        
    return;
}

/**
 * @brief Calculates the cosine similarity between two trigram profiles
 *
 * @param textProfile The text trigram profile
 * @param languageProfile The language trigram profile
 * @return float The cosine similarity score
 */
float getCosineSimilarity(TrigramProfile &textProfile, TrigramProfile &languageProfile)
{
    float sumCoincidende = 0;
    for(auto &textTriagram : textProfile)
    {
        if(languageProfile.find(textTriagram.first) != languageProfile.end())
        {
             if(textTriagram.second >= 0.01 && textTriagram.second <=  0.9) // Keeps only trigrams with frequency between 1% and 90%
                sumCoincidende += textTriagram.second * languageProfile[textTriagram.first];
            else
                continue;
        } else 
        {
            continue;
        }
    }

    return sumCoincidende; 
}

/**
 * @brief Identifies the language of a text.
 *
 * @param text A Text (vector of lines)
 * @param languages A list of Language objects
 * @return string The language code of the most likely language
 */
string identifyLanguage(const Text &text, LanguageProfiles &languages)
{
    TrigramProfile triagramProfile = buildTrigramProfile(text);

    normalizeTrigramProfile(triagramProfile);

    float maxCosineSimilarity = 0;

    string identifiedLanguageCode = "---";

    for(auto &language : languages)
    {
        float cosineSimilarity = getCosineSimilarity(triagramProfile, language.trigramProfile);

        if(cosineSimilarity > maxCosineSimilarity)
        {
            maxCosineSimilarity = cosineSimilarity;
          
            identifiedLanguageCode = language.languageCode;
        }
    }

    return identifiedLanguageCode;
}
