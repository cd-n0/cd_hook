/**
 * @file ch_hook.h
 * @brief A hooking library for inline and VMT hooks.
 *
 * This library provides functionalities to hook functions at runtime using 
 * inline or VMT (Virtual Method Table) hooking mechanisms.
 */
#ifndef CH_HOOK_H_
#define CH_HOOK_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @defgroup Architecture-specific Macros
 * @brief Constants for instruction injection depending on the architecture.
 * @{
 */
#if defined(__x86_64__) || defined(_M_X64)
#define MOV_ACC 0x48, 0xB8 ///< x86_64: MOV instruction opcode for setting the accumulator register.
#define JUMP_ADDRESS_OFFSET 2 ///< Offset of jump address in jmp_bytes array for x86_64.
#define ADDRESS_PADDING 0, 0, 0, 0, 0, 0, 0, 0 ///< Padding for jump address.
#define INLINE_LENGTH 12 ///< Length of inline hook instructions for x86_64.
#elif defined(__i386__) || defined(_M_IX86)
#define MOV_ACC 0xB8 ///< i386: MOV instruction opcode for setting the accumulator register.
#define JUMP_ADDRESS_OFFSET 1 ///< Offset of jump address in jmp_bytes array for i386.
#define ADDRESS_PADDING 0, 0, 0, 0 ///< Padding for jump address.
#define INLINE_LENGTH 7 ///< Length of inline hook instructions for i386.
#else
#error This architecture is not supported
#endif

#define JMP_ACC 0xff, 0xe0 ///< JMP instruction opcode.
/**@}*/

/**
 * @enum ch_hook_type
 * @brief Specifies the type of hook applied.
 */
typedef enum ch_hook_type_e {
    CH_HOOK_UNDEFINED = 0, ///< No hook is applied.
    CH_HOOK_INLINE, ///< Inline hook
    CH_HOOK_VMT ///< VMT hook
} ch_hook_type;

/**
 * @enum ch_hook_errors
 * @brief Error codes for hook operations.
 */
typedef enum ch_hook_errors_e {
    CH_HOOK_OK = 0, ///< Operation completed successfully.
    CH_HOOK_ERROR_ALREADY_HOOKED, ///< Hook is already applied.
    CH_HOOK_ERROR_WRONG_HOOK_METHOD, ///< Hook type mismatch.
    CH_HOOK_ERROR_NOT_HOOKED, ///< Attempted to unhook but no hook is applied.
    CH_HOOK_ERROR_MEMORY_PROTECTION, ///< Failed to change memory protection.
    CH_HOOK_ERROR_UNDEFINED ///< Undefined or unknown error.
} ch_hook_errors;

/**
 * @struct ch_hook_ctx
 * @brief Context for managing hook data.
 *
 * This structure holds all necessary information to manage a hook,
 * including the original function, the hook function, and metadata
 * about the hook type and state.
 */
typedef struct {
    /**
     * @union hook_data
     * @brief Union to store hook-specific data.
     *
     * This union contains either:
     * - `old_bytes`: The original bytes of the hooked function (used in inline hooks).
     * - `vmt_index`: The index in the virtual method table (used in VMT hooks).
     */
    union {
        uint8_t old_bytes[INLINE_LENGTH]; ///< Original bytes overwritten by an inline hook.
        uint16_t vmt_index; ///< Index in the virtual method table for VMT hooks.
    } hook_data;

    void *to_hook; ///< Pointer to the function or method being hooked.
    void *original; ///< Pointer to the original function or method (pre-hooked state).
    void *hook; ///< Pointer to the replacement hook function.
    ch_hook_type type; ///< Type of the hook (e.g., inline or VMT).
    bool hooked; ///< Indicates whether the hook is currently applied.
} ch_hook_ctx;

/**
 * @brief Converts a status code to a human-readable string.
 * @param status The status code to convert.
 * @return A string representation of the status.
 */
const char *ch_error_to_string(const ch_hook_errors error);

/**
 * @brief Creates a hook context.
 * @param to_hook Pointer to the function or method to be hooked.
 * @param hook Pointer to the hook function.
 * @return Pointer to the created context or NULL on failure.
 */
ch_hook_ctx *ch_create_ctx(void *to_hook, void *hook);

/**
 * @brief Destroys a hook context and optionally unhooks it.
 * @param ctx The hook context to destroy.
 * @param unhook Whether to unhook before destroying the context.
 * @return Status of the operation.
 */
ch_hook_errors ch_destroy_ctx(ch_hook_ctx *ctx, const bool unhook);

/**
 * @brief Reinitializes an existing hook context.
 * @param ctx The context to reinitialize.
 * @param to_hook Pointer to the new function or method to hook.
 * @param hook Pointer to the new hook function.
 * @return Status of the operation.
 */
ch_hook_errors ch_reinitialize_ctx(ch_hook_ctx *ctx, void *to_hook, void *hook);

/**
 * @brief Initializes a hook context.
 * @param ctx The context to reinitialize.
 * @param to_hook Pointer to the function or method to hook.
 * @param hook Pointer to the hook function.
 * @return Status of the operation.
 */
void ch_initialize_ctx(ch_hook_ctx *ctx, void *to_hook, void *hook);

/**
 * @brief Restores a hook to it's original state before hooking.
 * @param ctx The hook context to unhook.
 * @return Status of the operation.
 */
ch_hook_errors ch_unhook(ch_hook_ctx *ctx);

/**
 * @brief Applies an inline hook.
 * @param ctx The hook context to apply the inline hook to.
 * @return Status of the operation.
 */
ch_hook_errors ch_inline(ch_hook_ctx *ctx);

/**
 * @brief Applies a VMT hook.
 * @param ctx The hook context to apply the VMT hook to.
 * @param vmt_index The index in the virtual method table to replace.
 * @return Status of the operation.
 */
ch_hook_errors ch_vmt(ch_hook_ctx *ctx, const size_t vmt_index);

/**
 * @brief Helper macro to call the original function of a hooked function.
 * @param CTX The hook context.
 * @param FUNCTION_PROTOTYPE original functions function prototype.
 * @param ... The arguments to pass to the original function.
 * 
 * It's better to not use a macro like this but it's good to have for testing
 * debugging etc. Works with C20+ C++20+ because of __VA_OPT__.
 */
#if __cplusplus >= 202002L || __STDC_VERSION__ >= 202311L
#define CH_CALL_ORIGINAL(CTX, FUNCTION_PROTOTYPE, ...)                        \
    do {                                                                      \
        if ((CTX)->hooked) {                                                  \
            ch_hook_type t = (CTX)->type;                                     \
            ch_unhook(CTX);                                                   \
            ((FUNCTION_PROTOTYPE)((CTX)->original))(__VA_OPT__(__VA_ARGS__)); \
            switch (t) {                                                      \
                case CH_HOOK_INLINE:                                          \
                    ch_inline(CTX);                                           \
                    break;                                                    \
                case CH_HOOK_VMT:                                             \
                    ch_vmt(CTX, (CTX)->hook_data.vmt_index);                  \
                    break;                                                    \
                default:                                                      \
                    break;                                                    \
            }                                                                 \
        }                                                                     \
    } while (0)
#else
#define CH_CALL_ORIGINAL(CTX, FUNCTION_PROTOTYPE, ...) static_assert(false, "Variable arguments isn't supported by your compiler/standard")
#endif


#ifdef __cplusplus
}
#endif

#endif /* CH_HOOK_H_ */
