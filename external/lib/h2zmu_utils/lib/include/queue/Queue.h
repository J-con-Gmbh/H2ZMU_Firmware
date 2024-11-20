//
// Created by jriessner on 03.07.23.
//

#ifndef UTILS_QUEUE_H
#define UTILS_QUEUE_H

#include <vector>
#include <map>
#include <mutex>
#include <functional>
#include <iostream>

#include "Item.h"
#include "subscription.h"
#include "epoch_time.h"

namespace utils {

    template<typename T>
    class Queue {
    private:
        std::map<u_int, Item<T>> q;
        u_int currentIndex = 0;
        u_int currentSubscription = 0;
        //Arp::Mutex q_mutex;
        //Arp::Mutex s_mutex;
        std::map<u_int, struct subscription> subscriptions;
        u_int maxItems = 10000;
        u_int maxSecondsBack = 86400;

        int saveIncrementIndex(int index) {
            index += 1;
            if (index >= this->maxItems) {
                index = 0;
            }

            return index;
        }

        u_int addToQueueMap(T item) {

            u_int index_saved_at;

            //if (this->q_mutex.TryLock()) {

                auto it = this->q.find(this->currentIndex);
                if (it == this->q.end()){
                    this->q.emplace(this->currentIndex, item);
                } else {
                    it->second = item;
                }
                index_saved_at = this->currentIndex;
            //}

            //if (this->s_mutex.TryLock()) {

                for (std::pair<const unsigned int, subscription> subscription: this->subscriptions) {

                    if ( (this->currentIndex == (subscription.second.index - 1))
                        || (this->currentIndex == (this->maxItems - 1) && (subscription.second.index == 0)) ) {

                        this->subscriptions[subscription.first].index += 1;

                    }
                }
            //}

            this->currentIndex = this->saveIncrementIndex(this->currentIndex);

            this->alertAll();

            return index_saved_at;
        }


        void alertAll() {
            for (auto &item: this->subscriptions) {
                //this->subscriptions[item.first].alert = true;
                item.second.alert = true;
            }
        }

    public:
        Queue<T>() = default;

        void setMaxAgeSeconds(u_int maxAge) {
            this->maxSecondsBack = maxAge;
        }

        void setMaxItems(u_int maxItemCount) {
            this->maxItems = maxItemCount;
        }

        u_int subscribe() {

            //this->s_mutex.Lock();

            this->subscriptions[this->currentSubscription] = {.id=this->currentSubscription, .index = this->currentIndex, .alert = false};
            u_int ret = this->currentSubscription;
            this->currentSubscription += 1;

            return ret;
        }

        u_int add(T item) {
            return this->addToQueueMap(item);
        }

        u_int add(std::vector<T> items) {
            u_int lastIndex;
            for (const auto &item: items) {
                lastIndex = this->addToQueueMap(item);
            }

            return lastIndex;
        }

        bool hasNew(u_int subscription) {
            return this->subscriptions[subscription].alert;
        }

        std::tuple<bool, T> getFromQueueMap(u_int index) {
            //std::lock_guard<std::mutex> lock_q(this->q_mutex);

            if (!this->q.count(index)) {
                return std::tuple<bool, T>(false, {});
            }

            Item<T> item = this->q.at(index);
            if ((item.getCreatedAt() + this->maxSecondsBack) < utils::epoch_time::getUnixTimestamp()) {
                return std::tuple<bool, T>(false, {});
            }

            std::tuple<bool, T> ret(true, item.getPayload());

            return ret;
        }

        std::tuple<bool, T> getNext(u_int subscription) {

            if (!this->subscriptions.count(subscription)) {
                return std::tuple<bool, T>(false, {});
            }
            if (!this->hasNew(subscription)) {
                return std::tuple<bool, T>(false, {});
            }

            struct subscription &sub = std::ref(this->subscriptions[subscription]);
            u_int &index = std::ref(sub.index);

            if (!this->q.count(index)) {
                return std::tuple<bool, T>(false, {});
            }
            if (index == this->currentIndex) {
                return std::tuple<bool, T>(false, {});
            }

            std::tuple<bool, T> ret = this->getFromQueueMap(index);

            sub.alert = (!this->isUpToDate(sub.id));

            index = this->saveIncrementIndex(index);

            return ret;
        }

        std::vector<T> getNew(u_int subscription) {
            //std::lock_guard<std::mutex> lock(this->s_mutex);

            std::vector<T> ret = {};
            if (!this->subscriptions.count(subscription)) {
                return ret;
            }
            if (!this->hasNew(subscription)) {
                return ret;
            }

            struct subscription &sub = std::ref(this->subscriptions[subscription]);
            u_int &index = sub.index;
            int i;
            for (i = index; i != this->currentIndex; i = this->saveIncrementIndex(i)) {
                std::tuple<bool, T> item = this->getFromQueueMap(i);
                if (std::get<bool>(item)) {
                    ret.emplace_back(std::get<T>(item));
                }
            }

            sub.alert = (!this->isUpToDate(sub.id));
            index = i;

            return ret;
        }

        bool isUpToDate(u_int subscription) {

            u_int &index = std::ref(this->subscriptions[subscription].index);

            return (this->currentIndex == index);
        }

    };

}

#endif //UTILS_QUEUE_H
