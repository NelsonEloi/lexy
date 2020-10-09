// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_DSL_ERROR_HPP_INCLUDED
#define LEXY_DSL_ERROR_HPP_INCLUDED

#include <lexy/_detail/type_name.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/condition.hpp>
#include <lexy/dsl/option.hpp>

namespace lexyd
{
template <typename Tag, typename Pattern>
struct _err : rule_base
{
    static constexpr auto has_matcher = true;

    struct matcher
    {
        template <typename Reader>
        LEXY_DSL_FUNC bool match(Reader&)
        {
            return false;
        }
    };

    template <typename NextParser>
    struct parser
    {
        template <typename Handler, typename Reader, typename... Args>
        LEXY_DSL_FUNC auto parse(Handler& handler, Reader& reader, Args&&...) ->
            typename Handler::result_type
        {
            if constexpr (!std::is_same_v<Pattern, void>)
            {
                if (auto begin = reader.cur(); Pattern::matcher::match(reader))
                    return LEXY_MOV(handler).error(reader,
                                                   lexy::error<Reader, Tag>(begin, reader.cur()));
            }

            return LEXY_MOV(handler).error(reader, lexy::error<Reader, Tag>(reader.cur()));
        }
    };

    /// Adds a pattern whose match will be part of the error location.
    template <typename P>
    LEXY_CONSTEVAL auto operator()(P) const
    {
        static_assert(lexy::is_pattern<P>);
        return _err<Tag, P>{};
    }
};

/// Matches nothing, produces an error with the given tag.
template <typename Tag>
constexpr auto error = _err<Tag, void>{};
} // namespace lexyd

namespace lexyd
{
/// Requires that lookahead will match a pattern at a location.
template <typename Tag, typename Pattern>
LEXY_CONSTEVAL auto require(Pattern pattern)
{
    // If we don't get the pattern, we create a failure.
    // Otherwise, we match the empty string.
    return opt(unless(pattern) >> error<Tag>);
}

/// Requires that lookahead does not match a pattern at a location.
template <typename Tag, typename Pattern>
LEXY_CONSTEVAL auto prevent(Pattern pattern)
{
    // Same as above, but we don't want to match the pattern.
    return opt(if_(pattern) >> error<Tag>);
}
} // namespace lexyd

#endif // LEXY_DSL_ERROR_HPP_INCLUDED
