/*
 * Copyright (c) 2022 Valentin B.
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

#include "assert.hpp"
#include "ptor_defines.hpp"
#include "ptor_types.hpp"
#include "util/util_encoding.hpp"
#include "util/util_literals.hpp"

namespace ptor::io {

    class BinaryBuffer {
        P_DISALLOW_COPY_AND_ASSIGN(BinaryBuffer);

    public:
        static constexpr size_t DefaultCapacity = 4_KB;

    private:
        /* Memory view in bytes on which we operate. */
        u8 *m_ptr;
        u8 *m_cursor;
        size_t m_capacity;

        /* The bit offset into the current cursor byte. */
        u8 m_bit_offset;

        /* Whether the above byte view is owned by this object or not. */
        bool m_managed;

    public:
        /* Constructs a BinaryBuffer which manages its own memory allocation. */
        explicit BinaryBuffer(size_t capacity = DefaultCapacity);

        /* Construct a BinaryBuffer over a borrowed byte view. */
        BinaryBuffer(uint8_t *buf, size_t size);

        ~BinaryBuffer();

        /* General buffer management. */

        P_ALWAYS_INLINE bool IsManaged() const { return m_managed; }

        P_ALWAYS_INLINE u8 *GetCursorPtr() { return m_cursor; }
        P_ALWAYS_INLINE const u8 *GetCursorPtr() const { return m_cursor; }

        P_ALWAYS_INLINE ptrdiff_t GetCursorOffset() const {
            return m_cursor - m_ptr;
        }

        P_ALWAYS_INLINE u8 *GetOffsetPtr(ptrdiff_t offset) {
            P_DEBUG_ASSERT(offset >= 0 && offset <= GetCursorOffset());
            return m_ptr + offset;
        }

        P_ALWAYS_INLINE const u8 *GetOffsetPtr(ptrdiff_t offset) const {
            P_DEBUG_ASSERT(offset >= 0 && offset <= GetCursorOffset());
            return m_ptr + offset;
        }

        P_ALWAYS_INLINE size_t GetRemainingBytes() const {
            P_DEBUG_ASSERT(m_ptr <= m_cursor && m_cursor <= m_ptr + m_capacity);
            return static_cast<size_t>((m_ptr + m_capacity) - m_cursor);
        }

        P_ALWAYS_INLINE size_t GetRemainingBits() const {
            return (this->GetRemainingBytes() * BITSIZEOF(u8)) + (BITSIZEOF(u8) - m_bit_offset);
        }

        P_ALWAYS_INLINE size_t GetPassedBytes() const {
            P_DEBUG_ASSERT(m_ptr <= m_cursor && m_cursor <= m_ptr + m_capacity);
            return static_cast<size_t>(GetCursorOffset());
        }

        P_ALWAYS_INLINE size_t GetPassedBits() const {
            return (this->GetPassedBytes() * BITSIZEOF(u8)) + m_bit_offset;
        }

        P_ALWAYS_INLINE void RewindCursor(ptrdiff_t offset = 0) {
            auto *rewound = m_ptr + offset;
            P_DEBUG_ASSERT(m_ptr <= rewound && rewound <= m_cursor);
            m_cursor = rewound;
        }

        P_ALWAYS_INLINE bool HasSpaceForBytes(size_t nbytes) const {
            return this->GetRemainingBytes() >= nbytes;
        }

        P_ALWAYS_INLINE bool HasSpaceForBits(size_t nbits) const {
            return this->GetRemainingBits() >= nbits;
        }

        void Grow(size_t new_capacity);

        /* Binary serialization and deserialization. */

    private:
        P_ALWAYS_INLINE void RealignCursorToByte() {
            if (m_bit_offset != 0) {
                m_cursor += 1;
                m_bit_offset = 0;
            }
        }

        P_ALWAYS_INLINE void AdvanceCursorByBits(size_t nbits) {
            /* Try to advance the cursor by full bytes first. */
            m_cursor += (nbits / BITSIZEOF(u8));

            /* Add the remaining bits and account for byte overflow. */
            m_bit_offset += (nbits & (BITSIZEOF(u8) - 1));
            if (m_bit_offset >= BITSIZEOF(u8)) {
                m_cursor += 1;
                m_bit_offset = 0;
            }
        }

    public:
        template <std::integral T, std::endian BO = std::endian::little>
        T ReadValue() {
            static_assert(!std::is_same_v<T, bool>, "use ReadBit() instead");

            /* We start reading full bytes at aligned byte boundary. */
            RealignCursorToByte();

            /* Check if we have enough space to read the requested value. */
            constexpr size_t ReadSize = sizeof(T);
            P_ASSERT(this->HasSpaceForBytes(ReadSize), "buffer too short to read {} more bytes", ReadSize);

            /* Read the value out of the buffer. */
            const T value = util::Decode<T, BO>(m_cursor);
            m_cursor += ReadSize;
            return value;
        }

        template <std::integral T, std::endian BO = std::endian::little>
        void WriteValue(const T value) {
            static_assert(!std::is_same_v<T, bool>, "use WriteBit() instead");

            /* We start reading full bytes at aligned byte boundary. */
            RealignCursorToByte();

            /* If we don't have enough space to write, allocate more. */
            constexpr size_t WriteSize = sizeof(T);
            if (this->HasSpaceForBytes(WriteSize)) {
                this->Grow(m_capacity * 2);
            }

            /* Write the value to the buffer. */
            util::Encode<T, BO>(m_cursor, value);
            m_cursor += WriteSize;
        }

        void ReadBytes(void *out, size_t len);

        void WriteBytes(const void *in, size_t len);

        bool ReadBit();

        void WriteBit(bool value);

        u32 ReadBits(size_t len);

        void WriteBits(u32 value, size_t len);
    };

}
