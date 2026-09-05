
package com.mtk.map;

/**
 * Mix пучок (WorldWideMix §5, §14).
 *
 * Occupied base pages of one document: next/prev stay inside this chain
 * and do not walk into another document on the same tree. A hole in the
 * parent Branch is not a member; only linked leaves are.
 *
 * The even tree kept every leaf in one absolute next/prev list. A
 * {@link Leaf#cutAfter()} / {@link Leaf#cutBefore()} splits that list
 * into two bunches — two documents on one Branch tree. Growth from an
 * arbitrary vertex ({@link #growFrom(Branch)}) starts a new isolated
 * chain instead of appending to the main one.
 *
 * @author <a href="mailto:mtkravchenko@gmail.com">Mikhail Kravchenko</a>
 */
public class Bunch {
    final Leaf first;

    public Bunch(Leaf member) {
        if (member == null)
            throw new NullPointerException("bunch member");
        this.first = member.bunchStart();
    }

    public Leaf first() {
        return first;
    }

    public Leaf last() {
        return first.bunchEnd();
    }

    public boolean contains(Leaf leaf) {
        return first.sameBunch(leaf);
    }

    /**
     * Split this document after {@code leaf}. The tail becomes another
     * bunch on the same tree.
     */
    public Bunch cutAfter(Leaf leaf) {
        if (leaf == null || !contains(leaf))
            throw new IllegalArgumentException("leaf is not in this bunch");
        Leaf tail = leaf.getNext();
        leaf.cutAfter();
        if (tail == null)
            return null;
        return new Bunch(tail);
    }

    /**
     * Split this document before {@code leaf}. This bunch keeps the tail
     * starting at {@code leaf}; the head is returned as the other document.
     */
    public Bunch cutBefore(Leaf leaf) {
        if (leaf == null || !contains(leaf))
            throw new IllegalArgumentException("leaf is not in this bunch");
        Leaf head = leaf.getPrev();
        leaf.cutBefore();
        if (head == null)
            return null;
        return new Bunch(head);
    }

    /**
     * Grow a new isolated bunch from {@code vertex} — any Branch, not only
     * the document root. The new leaf sits in the tree; its next/prev are
     * not joined to any existing chain.
     */
    public static Bunch growFrom(Branch vertex) {
        if (vertex == null)
            throw new NullPointerException("vertex");
        return new Bunch(vertex.newBunch());
    }

    public static Bunch growFrom(Branch vertex, int hopLength) {
        if (vertex == null)
            throw new NullPointerException("vertex");
        return new Bunch(vertex.newBunch(hopLength));
    }
}
