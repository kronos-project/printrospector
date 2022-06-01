/*
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

#include "util/util_binary_buffer.hpp"

namespace ptor::util {

    BinaryBuffer::BinaryBuffer(const size_t capacity)
        : m_ptr{nullptr}, m_cursor{nullptr}, m_capacity{capacity}, m_bit_offset{0}, m_managed{true}
    {
        if (capacity != 0) {
            m_ptr    = new u8[capacity]();
            m_cursor = m_ptr;
        }
    }

    BinaryBuffer::~BinaryBuffer() {
        if (m_managed) {
            delete[] m_ptr;
        }
    }

    void BinaryBuffer::Grow(size_t new_capacity) {
        if (m_capacity < new_capacity) {
            /* Back up the current cursor offset to restore it later. */
            const auto cursor_offset = this->GetCursorOffset();

            /* Allocate a new buffer and copy previous contents over. */
            auto *new_ptr = new u8[new_capacity]();
            std::memcpy(new_ptr, m_ptr, m_capacity);
            delete[] m_ptr;

            /* Set the new buffer state. */
            m_ptr      = new_ptr;
            m_cursor   = m_ptr + cursor_offset;
            m_capacity = new_capacity;
        }
    }

    void BinaryBuffer::ReadBytes(void *out, size_t len) {
        /* We start reading full bytes at aligned byte boundary. */
        RealignCursorToByte();

        /* Check if we have enough space to read the requested bytes. */
        P_ASSERT(this->HasSpaceForBytes(len), "buffer too short to read {} more bytes", len);

        /* Copy bytes out of the buffer. */
        std::memcpy(out, m_cursor, len);
        m_cursor += len;
    }

    void BinaryBuffer::WriteBytes(const void *in, size_t len) {
        /* We start reading full bytes at aligned byte boundary. */
        RealignCursorToByte();

        /* If we don't have enough space to write, allocate more. */
        if (this->HasSpaceForBytes(len)) {
            this->Grow(m_capacity * 2);
        }

        /* Copy the bytes to the buffer. */
        std::memcpy(m_cursor, in, len);
        m_cursor += len;
    }

    bool BinaryBuffer::ReadBit() {
        /* Check if we have enough space to read one bit. */
        P_ASSERT(this->HasSpaceForBits(1), "buffer too short to read one bit");

        /* Extract the bit out of the buffer. */
        const bool value = (*m_cursor & P_BIT(m_bit_offset)) != 0;
        AdvanceCursorByBits(1);
        return value;
    }

    void BinaryBuffer::WriteBit(bool value) {
        /* If we don't have enough space to write, allocate more. */
        if (this->HasSpaceForBits(1)) {
            this->Grow(m_capacity * 2);
        }

        /* Write the bit to the buffer. */
        *m_cursor |= (static_cast<u8>(value) << m_bit_offset);
        AdvanceCursorByBits(1);
    }

    u32 BinaryBuffer::ReadBits(size_t len) {
        u32 out = 0;

        /* Check if we have enough space to read one bit. */
        P_ASSERT(this->HasSpaceForBits(len), "buffer too short to read {} bits", len);

        size_t count = 0;
        while (len != 0) {
            /* Determine how many bits fit in the current cursor byte. */
            const size_t current_bits = std::min(BITSIZEOF(u8) - m_bit_offset, len);

            /* Extract the remaining bits from the current byte. */
            u32 masked = (*m_cursor & P_MASK(m_bit_offset, m_bit_offset + current_bits)) >> m_bit_offset;
            out |= masked << count;

            /* Update bit counts for the next round of processing. */
            count += current_bits;
            len   -= current_bits;

            /* Advance the cursor by the read amount of bits. */
            this->AdvanceCursorByBits(current_bits);
        }

        return out;
    }

    void BinaryBuffer::WriteBits(const u32 value, size_t len) {
        /* If we don't have enough space to write, allocate more. */
        if (this->HasSpaceForBits(len)) {
            this->Grow(m_capacity * 2);
        }

        size_t count = 0;
        while (len != 0) {
            /* Determine how many bits fit in the current cursor byte. */
            const size_t current_bits = std::min(BITSIZEOF(u8) - m_bit_offset, len);

            /* Commit the determined number of bits onto the current byte. */
            const u8 masked = (value & P_MASK(count, count + current_bits)) >> count;
            *m_cursor = (*m_cursor & ~P_MASK(m_bit_offset, m_bit_offset + current_bits)) | (masked << m_bit_offset);

            /* Update bit counts for the next round of processing. */
            count += current_bits;
            len   -= current_bits;

            /* Advance the cursor by the read amount of bits. */
            this->AdvanceCursorByBits(current_bits);
        }
    }

}
