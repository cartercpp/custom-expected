#pragma once

#include <utility>
#include <type_traits>
#include <memory>
#include <concepts>

template <typename ErrorType>
struct unexpected
{
	ErrorType m_error;
};

template <typename T>
unexpected(T) -> unexpected<T>;

template <typename ValueType, typename ErrorType>
class expected;

template <typename T>
constexpr bool is_expected_v = false;

template <typename V, typename T>
constexpr bool is_expected_v<expected<V, T>>
				= true;

template <typename ValueType, typename ErrorType>
	requires (!std::same_as<ValueType, ErrorType>
			&& !std::is_reference_v<ValueType>
			&& !std::is_reference_v<ErrorType>)
class expected<ValueType, ErrorType>
{
public:

	using value_type = ValueType;
	using error_type = ErrorType;

	constexpr expected()
		requires std::is_default_constructible_v<
										ValueType>
		: m_value{}, m_hasValue{ true }
	{
	}

	constexpr expected(const unexpected<ErrorType>& e)
		: m_error{ e.m_error }, m_hasValue{ false }
	{
	}

	constexpr expected(unexpected<ErrorType>&& e)
		: m_error{ std::move(e.m_error) },
		m_hasValue{ false }
	{
	}

	constexpr expected(const ValueType& value)
		: m_value{ value }, m_hasValue{ true }
	{
	}

	constexpr expected(ValueType&& value)
		noexcept
		: m_value{ std::move(value) },
		m_hasValue{ true }
	{
	}

	constexpr expected(
		const expected<ValueType, ErrorType>& other
	)
	{
		if (other)
			std::construct_at(
				&m_value, other.m_value
			);
		else
			std::construct_at(
				&m_error, other.m_error
			);

		m_hasValue = other.m_hasValue;
	}

	constexpr expected(
		expected<ValueType, ErrorType>&& other
	)
	{
		if (other)
			std::construct_at(
				&m_value, std::move(other.m_value)
			);
		else
			std::construct_at(
				&m_error, std::move(other.m_error)
			);

		m_hasValue = other.m_hasValue;
	}

	constexpr auto& operator=(
		const expected<ValueType, ErrorType>& other
	)
	{
		if (m_hasValue && other)
			m_value = other.value();
		else if (m_hasValue && !other)
		{
			std::destroy_at(&m_value);
			std::construct_at(&m_error, other.error());
		}
		else if (!m_hasValue && other)
		{
			std::destroy_at(&m_error);
			std::construct_at(&m_value, other.value());
		}
		else
			m_error = other.error();

		return *this;
	}

	constexpr auto& operator=(
		expected<ValueType, ErrorType>&& other
	) noexcept
	{
		if (m_hasValue && other)
			m_value = std::move(other.value());
		else if (m_hasValue && !other)
		{
			std::destroy_at(&m_value);
			std::construct_at(&m_error,
							std::move(other.error()));
		}
		else if (!m_hasValue && other)
		{
			std::destroy_at(&m_error);
			std::construct_at(&m_value,
							std::move(other.value()));
		}
		else
			m_error = std::move(other.error());

		return *this;
	}

	constexpr ~expected()
	{
		if (m_hasValue)
			std::destroy_at(&m_value);
		else
			std::destroy_at(&m_error);
	}

	constexpr ValueType& value()&
	{
		return m_value;
	}

	constexpr const ValueType& value() const&
	{
		return m_value;
	}

	constexpr ValueType&& value()&&
	{
		return std::move(m_value);
	}

	constexpr const ValueType&& value() const&&
	{
		return static_cast<const ValueType&&>(
			m_value
		);
	}

	constexpr bool has_value() const noexcept
	{
		return m_hasValue;
	}

	constexpr operator bool() const noexcept
	{
		return m_hasValue;
	}

	constexpr ValueType value_or(
		const ValueType& other
	) const noexcept
	{
		return m_hasValue ? m_value : other;
	}

	template <typename... Ts> requires
		std::is_constructible_v<ValueType, Ts...>
	constexpr void emplace(Ts&&... args)
	{
		if (m_hasValue)
			std::destroy_at(&m_value);
		else
			std::destroy_at(&m_error);

		std::construct_at(&m_value,
						std::forward<Ts>(args)...);
		m_hasValue = true;
	}

	constexpr ErrorType& error() &
	{
		return m_error;
	}

	constexpr const ErrorType& error() const&
	{
		return m_error;
	}

	constexpr ErrorType&& error() &&
	{
		return std::move(m_error);
	}

	constexpr const ErrorType&& error() const &&
	{
		return static_cast<const ErrorType&&>(
			m_error
		);
	}

	constexpr ErrorType error_or(
		const ErrorType& other
	) const noexcept
	{
		return !m_hasValue ? m_error : other;
	}

	template <typename Callable>
	constexpr auto transform(Callable&& callable)&
	{
		using NewValueType = std::remove_cv_t<
			std::invoke_result_t<Callable, ValueType&>
		>;

		if (m_hasValue)
			return expected<NewValueType, ErrorType>{
				std::forward<Callable>(callable)(m_value)
			};
		else
			return expected<NewValueType, ErrorType>{
				unexpected{ m_error }
			};
	}

	template <typename Callable>
	constexpr auto transform(Callable&& callable) const&
	{
		using NewValueType = std::remove_cv_t<
			std::invoke_result_t<Callable, const ValueType&>
		>;

		if (m_hasValue)
			return expected<NewValueType, ErrorType>{
				std::forward<Callable>(callable)(m_value)
			};
		else
			return expected<NewValueType, ErrorType>{
				unexpected{ m_error }
			};
	}

	template <typename Callable>
	constexpr auto transform(Callable&& callable)&&
	{
		using NewValueType = std::remove_cv_t<
			std::invoke_result_t<Callable, ValueType&&>
		>;

		if (m_hasValue)
			return expected<NewValueType, ErrorType>{
				std::forward<Callable>(callable)
				(std::move(m_value))
			};
		else
			return expected<NewValueType, ErrorType>{
				unexpected{ std::move(m_error) }
			};
	}

	template <typename Callable>
	constexpr auto transform(Callable&& callable)
		const &&
	{
		using NewValueType = std::remove_cv_t<
			std::invoke_result_t<Callable,
							const ValueType&&>
		>;

		if (m_hasValue)
			return expected<NewValueType, ErrorType>{
				std::forward<Callable>(callable)
				(static_cast<const ValueType&&>(
					m_value))
			};
		else
			return expected<NewValueType, ErrorType>{
				unexpected{
					static_cast<const ErrorType&&>(
						m_error)}
			};
	}

	template <typename Callable>
	constexpr auto and_then(Callable&& callable)&
	{
		using NewExpectedType = std::remove_cv_t<
			std::invoke_result_t<Callable, ValueType&>
		>;

		static_assert(
			is_expected_v<NewExpectedType>
			&& std::same_as<
				typename NewExpectedType::error_type,
				ErrorType
			>,
			"callable must return an expected"
			" w/the same error type"
		);

		if (m_hasValue)
			return NewExpectedType{
				std::forward<Callable>(callable)
				(m_value)
			};
		else
			return NewExpectedType{
				unexpected{m_error}
			};
	}

	template <typename Callable>
	constexpr auto and_then(Callable&& callable)
		const &
	{
		using NewExpectedType = std::remove_cv_t<
			std::invoke_result_t<Callable,
							const ValueType&>
		>;

		static_assert(
			is_expected_v<NewExpectedType>
			&& std::same_as<
				typename NewExpectedType::error_type,
				ErrorType
			>,
			"callable must return an expected"
			" w/the same error type"
		);

		if (m_hasValue)
			return NewExpectedType{
				std::forward<Callable>(callable)
				(m_value)
			};
		else
			return NewExpectedType{
				unexpected{m_error}
			};
	}

	template <typename Callable>
	constexpr auto and_then(Callable&& callable)&&
	{
		using NewExpectedType = std::remove_cv_t<
			std::invoke_result_t<Callable, ValueType&&>
		>;

		static_assert(
			is_expected_v<NewExpectedType>
			&& std::same_as<
				typename NewExpectedType::error_type,
				ErrorType
			>,
			"callable must return an expected"
			" w/the same error type"
		);

		if (m_hasValue)
			return NewExpectedType{
				std::forward<Callable>(callable)
				(std::move(m_value))
			};
		else
			return NewExpectedType{
				unexpected{std::move(m_error)}
			};
	}

	template <typename Callable>
	constexpr auto and_then(Callable&& callable)
		const &&
	{
		using NewExpectedType = std::remove_cv_t<
			std::invoke_result_t<Callable,
							const ValueType&&>
		>;

		static_assert(
			is_expected_v<NewExpectedType>
			&& std::same_as<
				typename NewExpectedType::error_type,
				ErrorType
			>,
			"callable must return an expected"
			" w/the same error type"
		);

		if (m_hasValue)
			return NewExpectedType{
				std::forward<Callable>(callable)
				(static_cast<const ValueType&&>(m_value))
			};
		else
			return NewExpectedType{
				unexpected{static_cast<const ErrorType&&>(
												m_error)}
			};
	}

	template <typename Callable>
	constexpr auto or_else(Callable&& callable)&
	{
		using NewExpectedType = std::remove_cv_t<
			std::invoke_result_t<Callable, ErrorType&>
		>;

		static_assert(
			is_expected_v<NewExpectedType>
			&& std::same_as<
				typename NewExpectedType::value_type,
				ValueType
			>,
			"callable must return an expected"
			" w/the same value type"
		);

		if (!m_hasValue)
			return std::forward<Callable>(callable)
										(m_error);
		else
			return NewExpectedType{m_value};
	}

	template <typename Callable>
	constexpr auto or_else(Callable&& callable)&&
	{
		using NewExpectedType = std::remove_cv_t<
			std::invoke_result_t<Callable, ErrorType&&>
		>;

		static_assert(
			is_expected_v<NewExpectedType>
			&& std::same_as<
				typename NewExpectedType::value_type,
				ValueType
			>,
			"callable must return an expected"
			" w/the same value type"
		);

		if (!m_hasValue)
			return std::forward<Callable>(callable)
								(std::move(m_error));
		else
			return NewExpectedType{
							std::move(m_value) };
	}

	template <typename Callable>
	constexpr auto or_else(Callable&& callable)
		const &
	{
		using NewExpectedType = std::remove_cv_t<
			std::invoke_result_t<Callable,
							const ErrorType&>
		>;

		static_assert(
			is_expected_v<NewExpectedType>
			&& std::same_as<
				typename NewExpectedType::value_type,
				ValueType
			>,
			"callable must return an expected"
			" w/the same value type"
		);

		if (!m_hasValue)
			return std::forward<Callable>(callable)
										(m_error);
		else
			return NewExpectedType{ m_value };
	}

	template <typename Callable>
	constexpr auto or_else(Callable&& callable)
		const &&
	{
		using NewExpectedType = std::remove_cv_t<
			std::invoke_result_t<Callable,
			const ErrorType&&>
		>;

		static_assert(
			is_expected_v<NewExpectedType>
			&& std::same_as<
				typename NewExpectedType::value_type,
				ValueType
			>,
			"callable must return an expected"
			" w/the same value type"
		);

		if (!m_hasValue)
			return std::forward<Callable>(callable)
						(static_cast<const ErrorType&&>(
												m_error));
		else
			return NewExpectedType{
				static_cast<const ValueType&&>(m_value)
			};
	}

private:

	union {
		ValueType m_value;
		ErrorType m_error;
	};
	bool m_hasValue;
};