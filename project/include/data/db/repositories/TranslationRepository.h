//
// Created by jriessner on 18.05.2022.
//

#ifndef H2ZMU_2_TRANSLATIONREPOSITORY_H
#define H2ZMU_2_TRANSLATIONREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/entities/e_Translation.h"
#include "data/db/DatabaseService.h"

class TranslationRepository: public Repository {
    std::map<int, struct translation> allTranslations;
    std::map<std::string, int> shortdescIdRel;

    static struct translation getTextblockFromQuery(std::string& query);
    void setTranslationFromQuery(std::string& query);
public:
    static std::shared_ptr<TranslationRepository> instance;

    TranslationRepository() : Repository("translations"){}
    bool loadAll() override;
    struct translation getTranslationById(int id);
    std::string getDefaultTranslationById(int id);
    bool addTranslation(std::string short_desc, std::string loc, std::string translation, bool overridePrevious = false);
    bool addTranslation(int id, std::string loc, std::string translation, bool overidePrevious = false);
};


#endif //H2ZMU_2_TRANSLATIONREPOSITORY_H