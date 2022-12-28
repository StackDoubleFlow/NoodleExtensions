#pragma once

#include <vector>

#include "tracks/shared/Animation/Track.h"

namespace NoodleExtensions {

    template<typename V, typename It, typename F>
    inline std::vector<V> find_all(It begin, It end, F&& pred) {
        std::vector<V> result;
        result.reserve(std::distance(end, begin));

        for (auto it = begin; it != end; it++) {
            if (pred(*it)) result.emplace_back(*it);
        }

        result.shrink_to_fit();

        return result;
    }

    template<typename List, typename V = typename List::value_type, typename F>
    inline auto find_all(List list, F&& pred) {
        return find_all<V>(std::begin(list), std::end(list), std::forward<F>(pred));
    }



    template<typename ToCast, typename It>
    inline std::vector<ToCast> of_type(It begin, It end) {
        std::vector<ToCast> result;
        result.reserve(std::distance(begin, end));

        for (auto it = begin; it != end; it++) {
            if (il2cpp_utils::AssignableFrom<ToCast>((*it)->klass)) result.emplace_back(static_cast<ToCast>(*it));
        }

        result.shrink_to_fit();

        return result;
    }

    template<typename ToCast, typename List>
    inline auto of_type(List list) {
        return of_type<ToCast>(std::begin(list), std::end(list));
    }

    inline std::optional<float> getTimeProp(std::span<Track*> tracks) {
        if (tracks.empty()) return {};

        Track const* timeTrack = nullptr;

        if (tracks.size() > 1) {
            auto trackIt = std::find_if(tracks.begin(), tracks.end(), [](Track const* track) {
                return track->properties.time.value.has_value();
            });

            if (trackIt != tracks.end()) {
                timeTrack = *trackIt;
            }
        } else {
            timeTrack = tracks.front();
        }

        if (!timeTrack) return std::nullopt;
        Property const& timeProperty = timeTrack->properties.time;
        if (!timeProperty.value) return std::nullopt;

        float time = timeProperty.value->linear;
        return time;
    }
}