#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

namespace rde
{
	template <typename Value, typename Key = uint64_t>
	class PackedArray
	{
	public:
		using KeyType = Key;
		using ValueType = Value;

	public:
		PackedArray() = default;
		~PackedArray() = default;

		bool Contains(KeyType key);

		bool AddValue(KeyType key, const ValueType& value);
		bool RemoveValue(KeyType key);

		auto GetValue(KeyType key) -> std::optional<ValueType>;
		bool SetValue(KeyType key, const ValueType& value);

		auto GetSize() const -> auto { return m_values.size(); }
		auto GetArray() const -> const auto& { return m_values; }

	private:
		using IndexType = size_t;

		std::vector<ValueType> m_values;
		std::unordered_map<IndexType, Key> m_keyMap;
		std::unordered_map<Key, IndexType> m_indexMap;
	};

	template <typename Value, typename Key>
	bool PackedArray<Value, Key>::Contains(KeyType key)
	{
		const auto indexIt = m_indexMap.find(key);
		return indexIt != m_indexMap.end();
	}

	template <typename Key, typename Value>
	bool PackedArray<Key, Value>::AddValue(KeyType key, const typename PackedArray<Key, Value>::ValueType& value)
	{
		m_values.push_back(value);
		const auto index = m_values.size() - 1;
		m_indexMap[key] = index;
		m_keyMap[index] = key;
		return true;
	}

	template <typename Key, typename Value>
	bool PackedArray<Key, Value>::RemoveValue(KeyType key)
	{
		const auto indexIt = m_indexMap.find(key);
		if (indexIt == m_indexMap.end())
			return false;

		const auto index = indexIt->second;
		m_keyMap.erase(index);
		m_indexMap.erase(key);

		if (index != m_values.size() - 1)
		{
			// It was not the last item that was removed,
			// so we need to move to last to fill the gap
			const auto toMoveIndex = m_values.size() - 1;
			const auto toMoveKey = m_keyMap[toMoveIndex];
			m_values[index] = m_values[toMoveIndex];
			m_keyMap[index] = toMoveKey;
			m_indexMap[toMoveKey] = index;
			m_values.resize(m_values.size() - 1);
		}

		return true;
	}

	template <typename Key, typename Value>
	auto PackedArray<Key, Value>::GetValue(KeyType key) -> std::optional<ValueType>
	{
		const auto indexIt = m_indexMap.find(key);
		if (indexIt == m_indexMap.end())
			return std::nullopt;

		const auto index = indexIt->second;
		const auto& value = m_values[index];
		return value;
	}

	template <typename Value, typename Key>
	bool PackedArray<Value, Key>::SetValue(KeyType key, const ValueType& value)
	{
		const auto indexIt = m_indexMap.find(key);
		if (indexIt == m_indexMap.end())
			return false;

		const auto index = indexIt->second;
		m_values[index] = value;
		return true;
	}

} // namespace rde