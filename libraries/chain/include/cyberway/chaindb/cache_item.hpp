#pragma once

#include <memory>

#include <cyberway/chaindb/common.hpp>
#include <cyberway/chaindb/object_value.hpp>

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

#include <boost/intrusive/set.hpp>
#include <boost/intrusive/list.hpp>

namespace cyberway { namespace chaindb {

    class  cache_object;
    struct cache_data;
    struct cache_index_value;
    struct cache_converter_interface;

    using cache_object_ptr = boost::intrusive_ptr<cache_object>;
    using cache_data_ptr   = std::unique_ptr<cache_data>;

    struct cache_data {
        virtual ~cache_data() = default;
    }; // struct cache_object_data

    struct cache_converter_interface {
        cache_converter_interface() = default;
        virtual ~cache_converter_interface() = default;

        virtual void convert_variant(cache_object&) const = 0;
    }; // struct cache_load_interface

    struct table_cache_map;

    struct cache_index_value final: public boost::intrusive::set_base_hook<> {
        const account_name      code;
        const account_name      scope;
        const table_name        table;
        const index_name        index;
        const std::vector<char> data;
        const cache_object&     object;

        cache_index_value() = default;
        cache_index_value(cache_index_value&&) = default;

        ~cache_index_value() = default;
    }; // struct cache_index_value

    class cache_object final:
        public boost::intrusive_ref_counter<cache_object>,
        public boost::intrusive::set_base_hook<>,
        public boost::intrusive::list_base_hook<>
    {
        table_cache_map* table_cache_map_ = nullptr;
        object_value     object_;
        cache_data_ptr   data_;
    public:
        cache_object(table_cache_map& map, object_value obj)
        : table_cache_map_(&map), object_(std::move(obj)) {
        }

        cache_object(cache_object&&) = default;

        ~cache_object() = default;

        bool is_deleted() const {
            return nullptr != table_cache_map_;
        }

        template <typename Request>
        bool is_valid_table(const Request& request) const {
            return
                object_.service.code  == request.code  &&
                object_.service.scope == request.scope &&
                object_.service.table == request.table;
        }

        void mark_deleted();

        void set_object(object_value);

        void set_revision(const revision_t rev) {
            object_.service.revision = rev;
        }

        template <typename T, typename... Args> void set_data(Args&&... args) {
            data_ = std::make_unique<T>(*this, std::forward<Args>(args)...);
        }

        primary_key_t pk() const {
            return object_.pk();
        }

        const object_value& object() const {
            return object_;
        }

        bool has_data() const {
            return !!data_;
        }

        template <typename T> T& get_data() const {
            assert(has_data());
            return *static_cast<T*>(data_.get());
        }
    }; // class cache_object

} } // namespace cyberway::chaindb
