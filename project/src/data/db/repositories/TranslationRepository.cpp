//
// Created by jriessner on 18.05.2022.
//

#include "data/db/repositories/TranslationRepository.h"

#include "data/db/repositories/ParamRepository.h"
#include "Log.h"

std::shared_ptr<TranslationRepository> TranslationRepository::instance;

bool TranslationRepository::loadAll() {
    std::string sql = "SELECT * FROM textblocks";
    std::string ret = databaseService->executeSqlReturn(sql);

    std::list<std::string> listSets = utils::strings::splitString(ret, ";");
    for (std::string &item : listSets) {
        struct translation trans = getTextblockFromQuery(item);
        allTranslations[trans.id] = trans;
        shortdescIdRel[trans.shrt] = trans.id;
    }

    sql = "SELECT * FROM translations";
    ret = databaseService->executeSqlReturn(sql);

    listSets = utils::strings::splitString(ret, ";");
    for (std::string &item : listSets) {
        setTranslationFromQuery(item);
    }

    return true;
}

struct translation TranslationRepository::getTextblockFromQuery(std::string &query) {
    struct translation trans;
    trans.id = stoi(utils::strings::getValueFromQuery(query, "id", ","));
    trans.shrt = utils::strings::getValueFromQuery(query, "short", ",");

    return trans;
}

void TranslationRepository::setTranslationFromQuery(std::string &query) {
    int id = stoi(utils::strings::getValueFromQuery(query, "id", ","));
    std::string loc = utils::strings::getValueFromQuery(query, "loc", ",");
    std::string trans = utils::strings::getValueFromQuery(query, "trans", ",");

    if (this->allTranslations.count(id)) {
        this->allTranslations[id].translations[loc] = trans;
    }
}

struct translation TranslationRepository::getTranslationById(int id) {
    for (const auto &translation: allTranslations)
    {
        if (translation.second.id == id)
        {
            return translation.second;
        }
    }

    return {};
}

std::string TranslationRepository::getDefaultTranslationById(int id) {
    if (!this->allTranslations.count(id)) {
        return "Translation TBD";
    }

    std::string defaultLanguage = ParamRepository::instance->getParamById(50).value;

    const struct translation &t = this->getTranslationById(id);
    std::string trans = t.translations.at(defaultLanguage);

    return trans;
}

bool TranslationRepository::addTranslation(std::string short_desc, std::string loc, std::string translation, bool overridePrevious) {
    return addTranslation(this->shortdescIdRel[short_desc], loc, translation, overridePrevious);
}

bool TranslationRepository::addTranslation(int id, std::string loc, std::string translation, bool overidePrevious) {
    if (!this->allTranslations.count(id)) {

        return false;
    }


    return false;
}