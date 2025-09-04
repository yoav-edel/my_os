#!/usr/bin/env sh
if [ -z "$STAGED" ]; then
exit 0
fi


# Helper: does any staged file match the regex?
matches() {
echo "$STAGED" | grep -E "$1" >/dev/null 2>&1
}


# 1) Format C/headers
if command -v clang-format >/dev/null 2>&1; then
FILES=$(echo "$STAGED" | grep -E '\\.(c|h|hpp|cpp|S)$' || true)
if [ -n "$FILES" ]; then
echo "[pre-commit] Running clang-format…"
# Format and restage
echo "$FILES" | xargs -I{} clang-format -i "{}"
echo "$FILES" | xargs -I{} git add "{}"
fi
else
echo "[pre-commit] clang-format not found; skipping formatting."
fi


# 2) Static analysis (light)
if command -v cppcheck >/dev/null 2>&1; then
FILES=$(echo "$STAGED" | grep -E '\\.(c|h)$' || true)
if [ -n "$FILES" ]; then
echo "[pre-commit] Running cppcheck…"
cppcheck --error-exitcode=1 \
--enable=warning,style,performance,portability \
--std=c99 --force --inline-suppr --quiet \
--suppress=missingIncludeSystem \
$(echo "$FILES")
fi
else
echo "[pre-commit] cppcheck not found; skipping static analysis."
fi


# 3) Build (fast)
if matches '\\.(c|h|S|asm|ld|mk|Makefile)$'; then
if [ -z "${SKIP_BUILD:-}" ]; then
if command -v make >/dev/null 2>&1; then
echo "[pre-commit] Building (make -s -j)…"
make -s -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)" || {
echo "[pre-commit] Build failed; aborting commit." >&2
exit 1
}
else
echo "[pre-commit] make not found; skipping build."
fi
else
echo "[pre-commit] SKIP_BUILD=1 set; skipping build."
fi
fi


# 4) QEMU smoke tests (optional)
if [ -z "${SKIP_QEMU:-}" ] && command -v qemu-system-i386 >/dev/null 2>&1; then
if [ -x scripts/run-qemu-test.sh ]; then
echo "[pre-commit] Running QEMU tests…"
if ! scripts/run-qemu-test.sh; then
echo "[pre-commit] QEMU test failed; aborting commit." >&2
exit 1
fi
else
echo "[pre-commit] scripts/run-qemu-test.sh not found; skipping QEMU tests."
fi
else
echo "[pre-commit] QEMU not available or SKIP_QEMU=1; skipping QEMU tests."
fi


exit 0