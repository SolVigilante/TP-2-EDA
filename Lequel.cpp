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
    unsigned int cantTrigramas = 0;


    wstring line = converter.from_bytes(text.front());

    for (int i =0; i< line.length()-3 && cantTrigramas <= 2000; i++)
    {
        if(line.length() < 3)
            continue;   

        string trigram = converter.to_bytes(line.substr(i,3));

        if(TrigramProfile.find(trigram) != TrigramProfile.end())
        {
            TrigramProfile[trigram] +=1;
        } else 
        {
            TrigramProfile[trigram] =1;
        }
        cantTrigramas++;
        
    }
    TrigramProfile["CantTrigramas"] = (float)cantTrigramas; //Guarda la cantidad de trigramas para normalizar despues

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
    float cantTrigramas = (int)trigramProfile["CantTrigramas"];

    for ( auto &triagramList : trigramProfile)
    {
        if(triagramList.second < cantTrigramas* 0.001 && triagramList.second > cantTrigramas*0.98 ) //Elimina los trigramas demasiado comunes que pertenecen a cualquier lenguaje y los menos comunes que no son represenativos
            trigramProfile.erase(triagramList.first);
        else
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
            sumCoincidende += textTriagram.second * languageProfile[textTriagram.first];
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
