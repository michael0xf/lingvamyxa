/*
 * Legacy hardcoded decisions used only as a compare-mode oracle while the
 * translator moves ANSI C surface rules into lm2/trans_registry.lmx tables.
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

static int lm_trans_legacy_is_reserved_head_name(LmP0Text name) {
    return
        lm_trans_text_equals(name, "L1") ||
        lm_trans_text_equals(name, "L2") ||
        lm_trans_text_equals(name, "fn") ||
        lm_trans_text_equals(name, "fm") ||
        lm_trans_text_equals(name, "sub") ||
        lm_trans_text_equals(name, "if") ||
        lm_trans_text_equals(name, "else") ||
        lm_trans_text_equals(name, "while") ||
        lm_trans_text_equals(name, "return") ||
        lm_trans_text_equals(name, "break") ||
        lm_trans_text_equals(name, "continue") ||
        lm_trans_text_equals(name, "end") ||
        lm_trans_text_equals(name, "until") ||
        lm_trans_text_equals(name, "synchronized") ||
        lm_trans_text_equals(name, "const") ||
        lm_trans_text_equals(name, "external");
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

static const char *lm_trans_legacy_expr_atom_payload(LmP0Text text) {
    if (lm_trans_text_equals(text, ".")) {
        return "emit.c-dot";
    }
    if (lm_trans_text_equals(text, "=")) {
        return "emit.==";
    }
    if (lm_trans_text_equals(text, "@")) {
        return "emit.&";
    }
    if (lm_trans_text_equals(text, "\\")) {
        return "emit.*-or-->";
    }
    return 0;
}

static const char *lm_trans_legacy_statement_payload(LmP0Text head) {
    if (lm_trans_text_equals(head, "return")) {
        return "return";
    }
    if (lm_trans_text_equals(head, "if")) {
        return "if";
    }
    if (lm_trans_text_equals(head, "while")) {
        return "while";
    }
    if (lm_trans_text_equals(head, "else")) {
        return "else";
    }
    if (lm_trans_text_equals(head, "synchronized")) {
        return "synchronized";
    }
    if (lm_trans_text_equals(head, "break")) {
        return "break";
    }
    if (lm_trans_text_equals(head, "continue")) {
        return "continue";
    }
    if (lm_trans_text_equals(head, "const")) {
        return "const";
    }
    if (lm_trans_text_equals(head, "[]")) {
        return "array-declaration";
    }
    return 0;
}

static const char *lm_trans_legacy_atom_statement_payload(LmP0Text atom) {
    if (lm_trans_text_equals(atom, "break")) {
        return "break";
    }
    if (lm_trans_text_equals(atom, "continue")) {
        return "continue";
    }
    if (lm_trans_text_equals(atom, "return")) {
        return "return";
    }
    return 0;
}

static const char *lm_trans_legacy_function_receiver_payload(LmP0Text head) {
    if (lm_trans_text_equals(head, "fn")) {
        return "function.primitive";
    }
    if (lm_trans_text_equals(head, "fm")) {
        return "function.struct";
    }
    if (lm_trans_text_equals(head, "sub")) {
        return "procedure";
    }
    return 0;
}

static const char *lm_trans_legacy_top_level_payload(LmP0Text head) {
    if (lm_trans_text_equals(head, "L1")) {
        return "lower.l1";
    }
    if (
        lm_trans_text_equals(head, "fn") ||
        lm_trans_text_equals(head, "fm") ||
        lm_trans_text_equals(head, "sub")
    ) {
        return "function";
    }
    if (lm_trans_text_equals(head, "external")) {
        return "external-function";
    }
    return 0;
}

static int lm_trans_legacy_frame_has_positional_name_argument(const LmP0Frame *frame) {
    if (frame == 0) {
        return 0;
    }

    return
        lm_trans_text_equals(frame->head, "fn") ||
        lm_trans_text_equals(frame->head, "fm") ||
        lm_trans_text_equals(frame->head, "sub") ||
        lm_trans_text_equals(frame->head, "synchronized") ||
        lm_trans_text_equals(frame->head, "[]") ||
        lm_trans_text_equals(frame->head, "entry");
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
