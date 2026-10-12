// Copyright (C) 2025-2026 356C LLC
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <utility>

namespace helix {

/// A crash-loop guard file that exists for exactly the lifetime of this object.
/// Wrap a call that can fault inside a GPU driver with it: every normal return,
/// a graceful failure included, removes the file, and only a process death inside
/// the scope (a driver SIGSEGV unwinds nothing) leaves it for the next startup to
/// promote into a persistent block (`Application`'s GPU guard check at startup).
class ScopedGuardFile {
  public:
    explicit ScopedGuardFile(std::string path) : path_(std::move(path)) {
        std::ofstream out(path_, std::ios::out | std::ios::trunc);
        armed_ = out.is_open() && (out << "1") && (out.close(), !out.fail());
    }

    ~ScopedGuardFile() {
        if (armed_) {
            std::error_code ec;
            std::filesystem::remove(path_, ec);
        }
    }

    ScopedGuardFile(const ScopedGuardFile&) = delete;
    ScopedGuardFile& operator=(const ScopedGuardFile&) = delete;

    /// False when the file could not be written: the scope runs unguarded.
    bool armed() const {
        return armed_;
    }

    const std::string& path() const {
        return path_;
    }

  private:
    std::string path_;
    bool armed_ = false;
};

} // namespace helix
