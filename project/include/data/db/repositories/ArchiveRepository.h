//
// Created by pemeriau on 9.11.2024.
//

#ifndef H2ZMU_2_ARCHIVEREPOSITORY_H
#define H2ZMU_2_ARCHIVEREPOSITORY_H

#include <map>

#include "Repository.h"
#include "data/db/DatabaseService.h"
#include "data/db/entities/e_Archive.h"


class ArchiveRepository : public Repository {
    std::map<int, struct archive> allArchives;

    private:

    public:
        static std::shared_ptr<ArchiveRepository> instance;

        ArchiveRepository() : Repository("archive") {};
        struct archive getArchiveById();
        bool loadAll() override;
};

#endif //H2ZMU_2_PARAMREPOSITORY_H
