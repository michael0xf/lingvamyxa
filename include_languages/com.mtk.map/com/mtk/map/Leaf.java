
package com.mtk.map;


import com.mtk.map.i.ILet;

/**
 * @author <a href="mailto:mtkravchenko@gmail.com">Mikhail Kravchenko</a>
 */
public class Leaf extends Branch {
    /** Default page length. Instance pages use {@link #getLength()}. */
    public static final int PAGE = Branch.LENGTH;

    /**
     * Mix пучок (WorldWideMix §5): next/prev only inside one bunch.
     * The even tree used to keep every leaf in one absolute chain.
     * A cut breaks that chain: two documents on the same Branch tree,
     * each with its own isolated next/prev.
     */
    Leaf next, prev;

    /**
     * Porting: caller already holds prev/this/next in address order.
     * Unlinks this from the net-prev list; does not take monitors.
     */
    void unlinkHeld() {
        Leaf p = this.prev;
        Leaf n = this.next;
        if (n == null) {
            if (p == null)
                return;
            p.next = null;
            this.prev = null;
        } else if (p == null) {
            n.prev = null;
            this.next = null;
        } else {
            n.prev = p;
            p.next = n;
            this.prev = null;
            this.next = null;
        }
    }

    // Porting: take prev/this/next, then unlink. Safe as a standalone delete.
    void delete(){
        final Leaf p = this.prev;
        final Leaf n = this.next;
        if (n == null && p == null)
            return;
        Branch.lockAddressChain(p, this, n, new Runnable() {
            public void run() {
                unlinkHeld();
            }
        });
    }

    // Porting: knot is the lower-address page; then this; then old next.
    void insertAfterOf(Leaf knot){
        if (knot == null)
            return;
        final Leaf k = knot;
        final Leaf n = knot.next;
        Branch.lockAddressChain(k, this, n, new Runnable() {
            public void run() {
                prev = k;
                if (n != null){
                    next = n;
                    k.next = Leaf.this;
                    n.prev = Leaf.this;
                }else{
                    k.next = Leaf.this;
                }
            }
        });
    }
    // Porting: old prev, then this, then knot (higher address).
    void insertBeforeOf(Leaf knot){
        if (knot == null)
            return;
        final Leaf k = knot;
        final Leaf p = knot.prev;
        Branch.lockAddressChain(p, this, k, new Runnable() {
            public void run() {
                next = k;
                if (p != null){
                    prev = p;
                    k.prev = Leaf.this;
                    p.next = Leaf.this;
                }else{
                    k.prev = Leaf.this;
                }
            }
        });
    }

    public  Object getLastElement(){
        Leaf leaf = this;
        while (leaf != null) {
            if (leaf.amount > 0)
                return leaf.arr[leaf.amount - 1];
            leaf = leaf.next;
        }
        return null;
    }

    public  Object getFirstElement(){
        Leaf leaf = this;
        while (leaf != null) {
            if (leaf.amount > 0)
                return leaf.arr[0];
            leaf = leaf.prev;
        }
        return null;
    }
    public  Object removeLastElement(){
        Leaf leaf = this;
        while (leaf != null) {
            int n;
            synchronized (leaf) {
                n = leaf.amount;
                if (n > 1)
                    return leaf.removeElementAt(n - 1);
            }
            if (n == 1)
                return leaf.removeElementAt(n - 1);
            Leaf nx = leaf.next;
            leaf.remove();
            leaf = nx;
        }
        return null;
    }

    public  Object removeFirstElement(){
        Leaf leaf = this;
        while (leaf != null) {
            int n;
            synchronized (leaf) {
                n = leaf.amount;
                if (n > 1)
                    return leaf.removeElementAt(0);
            }
            if (n == 1)
                return leaf.removeElementAt(0);
            Leaf p = leaf.prev;
            leaf.remove();
            leaf = p;
        }
        return null;
    }


    public int superCopyFrom(Object[] from, int offset, int toindex, int len){
        return super.copyFrom(from, offset, toindex, len);
    }

    public  int free(){
        return arr.length - amount;
    }

    @Override
    public synchronized int copyFrom(Object[] from, int offset, int toindex, int len) {
        int count = 0;
        if (len <= free()){
            count += superCopyFrom(from, offset, toindex, len);
        }else{
            ((Leaf)newNext()).superCopyFrom(arr, toindex, 0, amount - toindex);
            int firstLen = len;
            int free = free();
            if (firstLen > free)
                firstLen = free;
            count += superCopyFrom(from, offset, toindex, firstLen);
            len -= firstLen;
            offset += firstLen;
            if (len > 0){
                Leaf leaf = (Leaf)getOrNewNext();
                if (leaf.free() >= len){
                    count += leaf.superCopyFrom(from, offset, 0, len);
                }else{
                    int max = arr.length;
                    while (len > max){
                        leaf = (Leaf)leaf.newNext();
                        count += leaf.superCopyFrom(from, offset, 0, max);
                        len -= max;
                        offset += max;
                    }
                    leaf = (Leaf)leaf.getOrNewNext();
                    if (leaf.free() < len)
                        leaf = (Leaf)leaf.newNext();
                    count += leaf.superCopyFrom(from, offset, 0, len);
                }
            }
        }
        return count;
    }


    Leaf(BaseRoot tipTop){
        super(tipTop);
    }

    Leaf(BaseRoot tipTop, int hopLength){
        super(tipTop, hopLength);
    }

    Leaf(Branch node, int at){
        super(node, at);
    }

    Leaf(Branch node, int at, int hopLength){
        super(node, at, hopLength);
    }


    void setArray(Object[] args){
        int size = 0;
        for(int i = 0; i < args.length; i++){
            if (args[i] != null){
                if (i > size){
                    args[size] = args[i];
                }
                size++;
            }
        }
        Object[] page = args;
        int pageLen = getLength();
        if (args.length != pageLen) {
            page = new Object[pageLen];
            if (size > pageLen)
                size = pageLen;
            System.arraycopy(args, 0, page, 0, size);
        } else {
            for(int i = size; i < args.length; i++){
                args[i] = null;
            }
        }
        arr = page;
        amount = size;
    }


    public Leaf getPrev() {
        return prev;
    }

    public Leaf getNext() {
        return next;
    }

    /**
     * First leaf of this bunch (document). Stops at a cut; does not
     * walk into another document on the same tree.
     */
    public Leaf bunchStart() {
        Leaf p = this;
        while (p.prev != null)
            p = p.prev;
        return p;
    }

    /**
     * Last leaf of this bunch. Stops at a cut.
     */
    public Leaf bunchEnd() {
        Leaf n = this;
        while (n.next != null)
            n = n.next;
        return n;
    }

    public boolean sameBunch(Leaf other) {
        if (other == null)
            return false;
        return bunchStart() == other.bunchStart();
    }

    /**
     * Mix §5 first step: break next/prev after this leaf.
     * This becomes the last page of its document; the old next starts
     * another document on the same tree. Neighbours are not spliced.
     */
    public void cutAfter() {
        final Leaf n = this.next;
        if (n == null)
            return;
        Branch.lockAddressChain(this, n, null, new Runnable() {
            public void run() {
                if (Leaf.this.next != n)
                    return;
                n.prev = null;
                Leaf.this.next = null;
            }
        });
    }

    /**
     * Break next/prev before this leaf. This becomes the first page of
     * its document; the old prev ends the other document.
     */
    public void cutBefore() {
        final Leaf p = this.prev;
        if (p == null)
            return;
        Branch.lockAddressChain(p, this, null, new Runnable() {
            public void run() {
                if (Leaf.this.prev != p)
                    return;
                p.next = null;
                Leaf.this.prev = null;
            }
        });
    }

    /**
     * Grow a new isolated bunch as a sibling page under the same parent.
     * The new leaf is on the tree but not linked into this next/prev chain.
     */
    public synchronized Leaf newBunch() {
        return newBunch(getLength());
    }

    public synchronized Leaf newBunch(int hopLength) {
        return new Leaf(getOrNewNode(), getIndex() + 1, hopLength);
    }

    public synchronized Branch newNext() {

        Leaf leaf = new Leaf(getOrNewNode(), getIndex() + 1, getLength());
        leaf.insertAfterOf(this);
        return leaf;
    }

    // Porting: do not hold this across creating a lower-address sibling.
    // insertBeforeOf locks prev, then new leaf, then this.
    public Branch newPrev() {
        Leaf leaf = new Leaf(getOrNewNode(), getIndex(), getLength());
        leaf.insertBeforeOf(this);
        return leaf;
    }


    public Branch getOrNewPrev() {
        Leaf p = prev;
        if (p == null)
            return newPrev();
        return p;
    }

    @Override
    public synchronized Branch getOrNewNext() {
        if (next == null){
            return newNext();
        }
        return next;
    }



    @Override
    public Object removeElementAt(int index) {
        // Porting: last slot must take prev/this/next before dropping the page.
        synchronized (this) {
            if (amount != 1)
                return super.removeElementAt(index, false);
        }
        final int at = index;
        final Object[] box = new Object[1];
        final Leaf p = this.prev;
        final Leaf n = this.next;
        Branch.lockAddressChain(p, this, n, new Runnable() {
            public void run() {
                box[0] = Leaf.super.removeElementAt(at, false);
                if (amount == 0) {
                    Leaf.super.remove();
                    unlinkHeld();
                }
            }
        });
        return box[0];
    }

    @Override
    public void remove(){
        // Porting: one chain lock; unlinkHeld, not nested delete().
        final Leaf p = this.prev;
        final Leaf n = this.next;
        Branch.lockAddressChain(p, this, n, new Runnable() {
            public void run() {
                Leaf.super.remove();
                unlinkHeld();
            }
        });
    }


/*    @Override
    public Branch newBranch(){
        return new Leaf(mtkThread);
    }*/

    public ILet getValue(Key key, int index) {
        
        if (key == null)
            return null;
        return getRoot().getOrNewKeyDict(key).getLet(this, index);
    }


    
   /* public Object getObject(String key, int index) {
        
        if (key == null)
            return null;
        ILet ret = Key.getOrNew(key).getLet(this, index);
        if (ret != null)
            return ret.getValue();
        else
            return null;
    }*/

    
/*    public Object getTypeForKey(Class type, String key, int index) {
        ILet ret = getValue(Key.getOrNew(type.getName() + " " + key), index);
        if (ret != null)
            return ret.getValue();
        ret = getValue(Key.getOrNew(key), index);
        if (ret == null)
            return null;
        return Main.ins.getTypeForValue(type, ret.getValue());
    }

    public Object getTypeForKey(Class type, Key key, int index) {
        ILet ret = getValue(Key.getOrNew(type.getName() + " " + key.toString()), index);
        if (ret != null)
            return ret.getValue();
        ret = getValue(key, index);
        if (ret == null)
            return null;
        return Main.ins.getTypeForValue(type, ret.getValue());
    }*/


   /* class Ant extends Pointer{
          public Leaf getLeaf(){
              return Leaf.this;
          }
    }

    Pointer anchor(){
        if (anchor == null)
            anchor = new Pointer();
        return  anchor;
    }*/

}




/*
    public  int getIndex(int foundedLevel){
        return anchor().getIndex(foundedLevel);

    }

    public  void insertToBranch(Branch branch, int index){
        anchor().insertToBranch(branch, index);
    }

    public  void transferFromBranch(final Branch union, final int from, int end, final int toIndex){
        if (from >= union.size())
            return;
        if (end < from)
            return;
        if (end > union.size())
            end = union.size();
        int len = end - from;
        if (union instanceof Branch){
            Branch union1 = (Branch) union;
            len = copyFrom(union1.arr, from, toIndex, len);
        }else{
            for(int i = from, counter = 0; i < end; i++, counter++){
                final Object o = union.elementAt(i);
                insertElementAt(o, toIndex + i);
            }
        }
        if (len > 0)
            super.enumerate(toIndex, toIndex + len);
    }


*/