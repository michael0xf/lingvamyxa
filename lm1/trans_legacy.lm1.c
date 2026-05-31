/*
 * Legacy hardcoded decisions used only as a compare-mode oracle while the
 * translator moves ANSI C surface rules into lm2/trans_registry.lm4 tables.
 */

static const char *lm_trans_legacy_c_spelling(LmP0Text name) {
    if (lm_trans_text_equals(name, "u8")) {
        return "uint8_t";
    }
    if (lm_trans_text_equals(name, "u16")) {
        return "uint16_t";
    }
    if (lm_trans_text_equals(name, "u32")) {
        return "uint32_t";
    }
    if (lm_trans_text_equals(name, "u64")) {
        return "uint64_t";
    }
    if (lm_trans_text_equals(name, "i8")) {
        return "int8_t";
    }
    if (lm_trans_text_equals(name, "i16")) {
        return "int16_t";
    }
    if (lm_trans_text_equals(name, "i32")) {
        return "int32_t";
    }
    if (lm_trans_text_equals(name, "i64")) {
        return "int64_t";
    }

    return 0;
}

static int lm_trans_legacy_builtin_c_type_tail(LmP0Text name) {
    return
        lm_trans_legacy_c_spelling(name) != 0 ||
        lm_trans_text_equals(name, "void") ||
        lm_trans_text_equals(name, "char") ||
        lm_trans_text_equals(name, "short") ||
        lm_trans_text_equals(name, "int") ||
        lm_trans_text_equals(name, "long") ||
        lm_trans_text_equals(name, "float") ||
        lm_trans_text_equals(name, "double") ||
        lm_trans_text_equals(name, "signed") ||
        lm_trans_text_equals(name, "unsigned") ||
        lm_trans_text_equals(name, "size_t") ||
        lm_trans_text_equals(name, "FILE");
}

static int lm_trans_legacy_text_is_operator_atom(LmP0Text text) {
    return
        lm_trans_text_equals(text, "++") ||
        lm_trans_text_equals(text, "--") ||
        lm_trans_text_equals(text, "+") ||
        lm_trans_text_equals(text, "-") ||
        lm_trans_text_equals(text, "*") ||
        lm_trans_text_equals(text, "/") ||
        lm_trans_text_equals(text, "%") ||
        lm_trans_text_equals(text, "=") ||
        lm_trans_text_equals(text, "!=") ||
        lm_trans_text_equals(text, "<") ||
        lm_trans_text_equals(text, "<=") ||
        lm_trans_text_equals(text, ">") ||
        lm_trans_text_equals(text, ">=") ||
        lm_trans_text_equals(text, "&&") ||
        lm_trans_text_equals(text, "||") ||
        lm_trans_text_equals(text, "&") ||
        lm_trans_text_equals(text, "|") ||
        lm_trans_text_equals(text, "^") ||
        lm_trans_text_equals(text, "!") ||
        lm_trans_text_equals(text, "~") ||
        lm_trans_text_equals(text, "@") ||
        lm_trans_text_equals(text, "\\") ||
        lm_trans_text_equals(text, "[") ||
        lm_trans_text_equals(text, "]");
}

static int lm_trans_legacy_atom_is_prefix_expr_operator(LmP0Text text) {
    return
        lm_trans_text_equals(text, "++") ||
        lm_trans_text_equals(text, "--") ||
        lm_trans_text_equals(text, "+") ||
        lm_trans_text_equals(text, "-") ||
        lm_trans_text_equals(text, "@") ||
        lm_trans_text_equals(text, "\\") ||
        lm_trans_text_equals(text, "!") ||
        lm_trans_text_equals(text, "~");
}

static int lm_trans_legacy_atom_is_postfix_expr_operator(LmP0Text text) {
    return
        lm_trans_text_equals(text, "++") ||
        lm_trans_text_equals(text, "--");
}

static int lm_trans_legacy_atom_is_index_operator(LmP0Text text) {
    return lm_trans_text_equals(text, "[") || lm_trans_text_equals(text, "]");
}

static int lm_trans_legacy_frame_has_positional_name_argument(LmP0Text head) {
    return
        lm_trans_text_equals(head, "fn") ||
        lm_trans_text_equals(head, "fm") ||
        lm_trans_text_equals(head, "sub") ||
        lm_trans_text_equals(head, "synchronized") ||
        lm_trans_text_equals(head, "[]") ||
        lm_trans_text_equals(head, "entry");
}

static const char *lm_trans_legacy_function_receiver_payload(LmP0Text head) {
    if (lm_trans_text_equals(head, "fn")) {
        return "lm_trans_receiver_fn";
    }
    if (lm_trans_text_equals(head, "fm")) {
        return "lm_trans_receiver_fm";
    }
    if (lm_trans_text_equals(head, "sub")) {
        return "lm_trans_receiver_sub";
    }
    return 0;
}

static int lm_trans_legacy_atom_is_infix_expr_operator(
    LmP0Text text,
    const LmP0Node *operator_node,
    const LmP0Node *previous_operand
) {
    if (lm_trans_text_equals(text, "\\")) {
        return
            previous_operand != 0 &&
            lm_trans_nodes_touch(previous_operand, operator_node) &&
            !(previous_operand->kind == LM_P0_NODE_ATOM && lm_trans_atom_starts_string(previous_operand->as.atom));
    }

    return
        lm_trans_text_equals(text, "+") ||
        lm_trans_text_equals(text, "-") ||
        lm_trans_text_equals(text, "*") ||
        lm_trans_text_equals(text, "/") ||
        lm_trans_text_equals(text, "%") ||
        lm_trans_text_equals(text, "=") ||
        lm_trans_text_equals(text, "!=") ||
        lm_trans_text_equals(text, "<") ||
        lm_trans_text_equals(text, "<=") ||
        lm_trans_text_equals(text, ">") ||
        lm_trans_text_equals(text, ">=") ||
        lm_trans_text_equals(text, "&&") ||
        lm_trans_text_equals(text, "||") ||
        lm_trans_text_equals(text, "&") ||
        lm_trans_text_equals(text, "|") ||
        lm_trans_text_equals(text, "^");
}
