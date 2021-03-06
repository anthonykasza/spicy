// Copyright (c) 2020 by the Zeek Project. See LICENSE for details.

#pragma once

#include <hilti/ast/types/vector.h>
#include <hilti/base/uniquer.h>

#include <spicy/ast/aliases.h>
#include <spicy/ast/engine.h>
#include <spicy/ast/types/unit-item.h>
#include <spicy/ast/types/unit.h>

#include <utility>

namespace spicy::type::unit::item {

namespace detail {

static inline Type adaptType(Type type, const std::optional<Expression>& repeat) {
    if ( repeat )
        return type::Vector(type, type.meta());

    return type;
}

} // namespace detail

/** AST node for a unit field. */
class Field : public hilti::NodeBase, public spicy::trait::isUnitItem {
public:
    Field(const std::optional<ID>& id, Type type, Engine e, const std::vector<Expression>& args,
          std::optional<Expression> repeat, const std::vector<Expression>& sinks,
          std::optional<AttributeSet> attrs = {}, std::optional<Expression> cond = {}, std::vector<Hook> hooks = {},
          Meta m = Meta())
        : NodeBase(nodes((id ? id : _uniquer.get("anon")), detail::adaptType(std::move(type), repeat), node::none,
                         repeat, std::move(attrs), std::move(cond), args, sinks, std::move(hooks)),
                   std::move(m)),
          _is_anonynmous(! id.has_value()),
          _engine(e),
          _args_start(6),
          _args_end(_args_start + static_cast<int>(args.size())),
          _sinks_start(_args_end),
          _sinks_end(_sinks_start + static_cast<int>(sinks.size())) {}

    Field(const std::optional<ID>& id, Ctor ctor, Engine e, const std::vector<Expression>& args,
          std::optional<Expression> repeat, const std::vector<Expression>& sinks,
          std::optional<AttributeSet> attrs = {}, std::optional<Expression> cond = {}, std::vector<Hook> hooks = {},
          Meta m = Meta())
        : NodeBase(nodes((id ? id : _uniquer.get("anon")), detail::adaptType(ctor.type(), repeat), ctor, repeat,
                         std::move(attrs), std::move(cond), args, sinks, std::move(hooks)),
                   std::move(m)),
          _is_anonynmous(! id.has_value()),
          _engine(e),
          _args_start(6),
          _args_end(_args_start + static_cast<int>(args.size())),
          _sinks_start(_args_end),
          _sinks_end(_sinks_start + static_cast<int>(sinks.size())) {}

    Field(const std::optional<ID>& id, Item item, Engine e, const std::vector<Expression>& args,
          std::optional<Expression> repeat, const std::vector<Expression>& sinks,
          std::optional<AttributeSet> attrs = {}, std::optional<Expression> cond = {}, std::vector<Hook> hooks = {},
          const Meta& m = Meta())
        : NodeBase(nodes((id ? id : _uniquer.get("anon")), detail::adaptType(item.itemType(), repeat), item, repeat,
                         std::move(attrs), std::move(cond), args, sinks, std::move(hooks)),
                   m),
          _is_anonynmous(! id.has_value()),
          _engine(e),
          _args_start(6),
          _args_end(_args_start + static_cast<int>(args.size())),
          _sinks_start(_args_end),
          _sinks_end(_sinks_start + static_cast<int>(sinks.size())) {}

    auto id() const { return childs()[0].as<ID>(); }
    auto ctor() const { return childs()[2].tryAs<Ctor>(); }
    auto vectorItem() const { return childs()[2].tryAs<Item>(); }
    auto repeatCount() const { return childs()[3].tryAs<Expression>(); }
    auto attributes() const { return childs()[4].tryAs<AttributeSet>(); }
    auto condition() const { return childs()[5].tryAs<Expression>(); }
    auto arguments() const { return childs<Expression>(_args_start, _args_end); }
    auto sinks() const { return childs<Expression>(_sinks_start, _sinks_end); }
    auto hooks() const { return childs<Hook>(_sinks_end, -1); }
    Engine engine() const { return _engine; }

    auto isContainer() const { return parseType().isA<type::Vector>(); }
    auto isTransient() const { return _is_anonynmous; }

    Type parseType() const;

    Node& ctorNode() { return childs()[2]; }
    Node& vectorItemNode() { return childs()[2]; }
    Node& typeNode() { return childs()[1]; }

    bool operator==(const Field& other) const {
        return _engine == other._engine && id() == other.id() && _originalType() == other._originalType() &&
               attributes() == other.attributes() && arguments() == other.arguments() && sinks() == other.sinks() &&
               condition() == other.condition() && hooks() == other.hooks();
    }

    // Unit item interface
    Type itemType() const;
    auto isEqual(const Item& other) const { return node::isEqual(this, other); }

    // Node interface.
    auto properties() const { return node::Properties{{"engine", to_string(_engine)}}; }

private:
    Type _originalType() const { return child<Type>(1); }

    bool _is_anonynmous;
    Engine _engine;
    const int _args_start;
    const int _args_end;
    const int _sinks_start;
    const int _sinks_end;

    static inline util::Uniquer<ID> _uniquer;
};

} // namespace spicy::type::unit::item
