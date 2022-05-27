/*
 * Copyright (c)           Atmosphère-NX
 * Copyright (c) 2021-2022 Valentin B.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <functional>
#include <type_traits>
#include <utility>

#include "ptor_defines.hpp"

namespace ptor::util {

    /* https://github.com/Atmosphere-NX/Atmosphere/blob/master/libraries/libvapours/include/vapours/util/util_i_function.hpp */

    template <typename T>
    class IFunction;

    namespace impl {

        template <typename>
        struct GetFunctionTypeForObject;

        template <typename F, typename Ret, typename... Args>
        struct GetFunctionTypeForObject<Ret (F::*)(Args...)> {
            using Type = Ret(Args...);
        };

        template <typename F, typename Ret, typename... Args>
        struct GetFunctionTypeForObject<Ret (F::*)(Args...) const> {
            using Type = Ret(Args...);
        };

        template <typename>
        struct GetFunctionType;

        template <typename Ret, typename... Args>
        struct GetFunctionType<Ret(Args...)> {
            using Type = Ret(Args...);
        };

        template <typename Ret, typename... Args>
        struct GetFunctionType<Ret(*)(Args...)> : public GetFunctionType<Ret(Args...)> {};

        template <typename F>
        struct GetFunctionType<std::reference_wrapper<F>> : public GetFunctionType<F> {};

        template <typename F>
        struct GetFunctionType : public GetFunctionTypeForObject<decltype(&F::operator())> {};

        template <typename T, typename F, typename Enabled = void>
        class Function;

        template <typename Ret, typename... Args, typename F>
        class Function<Ret(Args...), F, std::enable_if_t<!(std::is_class_v<F> && !std::is_final_v<F>)>> final : public IFunction<Ret(Args...)> {
        private:
            F m_f;

        public:
            constexpr explicit Function(F f) : m_f(std::move(f)) {}
            constexpr ~Function() override {}

            constexpr Ret operator()(Args... args) const final {
                return m_f(std::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename... Args, typename F>
        class Function<Ret(Args...), F, std::enable_if_t<std::is_class_v<F> && !std::is_final_v<F>>> final : public IFunction<Ret(Args...)>, private F {
        public:
            constexpr explicit Function(F f) : F(std::move(f)) {}
            constexpr ~Function() override {}

            constexpr Ret operator()(Args... args) const final {
                return static_cast<const F &>(*this).operator()(std::forward<Args>(args)...);
            }
        };

        template <typename I, typename F>
        P_ALWAYS_INLINE constexpr auto MakeFunctionExplicitly(F f) {
            using FunctionType = Function<I, std::decay_t<F>>;
            return FunctionType(std::move(f));
        }

        template <typename I, typename T, typename R>
        P_ALWAYS_INLINE constexpr auto MakeFunctionExplicitly(R T::*f) {
            return MakeFunctionExplicitly<I>(std::mem_fn(f));
        }

    }

    template <typename Ret, typename... Args>
    class IFunction<Ret(Args...)> {
    protected:
        constexpr virtual ~IFunction() = default;

    public:
        constexpr virtual Ret operator()(Args...) const = 0;

        template <typename F>
        P_ALWAYS_INLINE static constexpr auto Make(F f) {
            return impl::MakeFunctionExplicitly<Ret(Args...)>(std::move(f));
        }
    };

}
