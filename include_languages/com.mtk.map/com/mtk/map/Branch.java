
package com.mtk.map;

/**
 * @author <a href="mailto:mtkravchenko@gmail.com">Mikhail Kravchenko</a>
 *
 * Porting (Lingvamyxa MIX / L2): Java monitors on the overlapping tree.
 * MapThreads used to serialize edits, so these locks were dropped. Restore
 * them so two writers on one live document are safe to port.
 *
 * Protocol:
 * 1. Default page is {@link #LENGTH} slots (32). Each Branch/Leaf may use
 *    any integer length (Mix path {@code 12.45.23432.7}). Packed
 *    {@link #getAddress(int)} is mixed-radix by those lengths; for 32
 *    it matches the old {@code << 5}.
 *    Page edit / slot re-sort: the monitor is the Leaf (this).
 * 2. Split / child re-sort: the monitor is that Branch, acquired bottom-up
 *    (page then parent), only up to the height that actually changes.
 *    Leaf delete unlinks prev/this/next in one chain lock; the last-element
 *    remove takes that chain before dropping the page.
 * 3. Two pages (cursor cross, prev/next links): lower address first
 *    (prev leaf, then next leaf).
 * 4. Do not hold these monitors across Bridge / Message Thread wait.
 * 5. A reader on another OS thread takes the same monitors, or does not
 *    walk the tree outside the owner thread's queue.
 */

public class Branch extends LiveVector implements LiveVector.Element{



    /** Default Mix RAM hop (~5 bits). Not a type of address. */
    final static int SIZE = 5;
    final static int LENGTH = 1 << SIZE;

    int length = LENGTH;

    public static int checkLength(int n) {
        if (n < 1)
            throw new IllegalArgumentException("Branch length " + n);
        return n;
    }

    public int getLength() {
        return length;
    }

    /**
     * Page capacity of this node, any positive integer. {@code 3.17} stays
     * {@code 3.17} when the page grows from 32 to 23433 slots.
     */
    public synchronized void setLength(int n) {
        n = checkLength(n);
        if (amount > n)
            throw new IndexOutOfBoundsException("occupied " + amount + " > " + n);
        if (length == n && arr != null && arr.length == n)
            return;
        Object[] next = new Object[n];
        if (arr != null && amount > 0)
            System.arraycopy(arr, 0, next, 0, amount);
        arr = next;
        length = n;
    }

    /**
     * Mix hole: raise {@link #amount} so {@code index} exists. Null slots
     * are not in the bunch. Does not shift neighbours.
     */
    public synchronized void ensureSlot(int index) {
        if (index < 0 || index >= getLength())
            throw new IndexOutOfBoundsException("slot " + index + " / " + getLength());
        if (arr == null)
            expand();
        if (index >= amount)
            amount = index + 1;
    }

    /**
     * Porting: lock two tree pages in address order (lower, then higher).
     * {@code lower == higher} or a null is skipped. Re-entrant if the caller
     * already holds one of them.
     */
    public static void lockLowerThenHigher(Object lower, Object higher, Runnable body) {
        if (higher == null || higher == lower) {
            if (lower == null) {
                body.run();
                return;
            }
            synchronized (lower) {
                body.run();
            }
            return;
        }
        if (lower == null) {
            synchronized (higher) {
                body.run();
            }
            return;
        }
        synchronized (lower) {
            synchronized (higher) {
                body.run();
            }
        }
    }

    /**
     * Porting: lock up to three pages already in address order (prev, this, next).
     */
    public static void lockAddressChain(Object a, Object b, Object c, Runnable body) {
        if (b == a)
            b = null;
        if (c == a || c == b)
            c = null;
        if (a == null) {
            lockLowerThenHigher(b, c, body);
            return;
        }
        if (b == null) {
            lockLowerThenHigher(a, c, body);
            return;
        }
        synchronized (a) {
            lockLowerThenHigher(b, c, body);
        }
    }


    void setArray(Object[] args){
        int size = 0;
        for(int i = 0; i < args.length; i++){
            if (args[i] != null){
                if (i > size){
                    args[size] = args[i];
                }
                size ++;
            }
        }
        for(int i = size; i < args.length; i++){
            args[i] = null;
        }
        arr = args;
        amount = size;
    }
    Branch(BaseRoot tipTop){
        this(tipTop, LENGTH);
    }

    Branch(BaseRoot tipTop, int hopLength){
        branch = tipTop;
        length = checkLength(hopLength);
        array(length);
    }

    Branch(Branch node, int at){
        this(node, at, node != null ? node.getLength() : LENGTH);
    }

    Branch(Branch node, int at, int hopLength){
        length = checkLength(hopLength);
        array(length);
        if (at < node.size()){
            node.insertElementAt(this, at);
        }else
            node.addElement(this);
    }

    @Override
    public synchronized void remove(){
        Branch branch = getPlace();
        if (branch != null)
            branch.removeElementAt(index);
    }

    public int getMaxBlockLength(){
        return getLength();
    }

    public synchronized Branch getOrNewNode(){
        Branch node = getPlace();
        if (node == null){
            node = newNode();
            node.addElement(this);
        }
        return node;
    }

    @Override
    public synchronized Object removeElementAt(int index){
          return removeElementAt(index, true);
    }

    /**
     * Porting: {@code dropEmpty} is false when a Leaf must unlink prev/next
     * without taking a lower-address monitor while only holding this.
     */
    synchronized Object removeElementAt(int index, boolean dropEmpty){
          Object ret = super.removeElementAt(index);
          if (dropEmpty && amount == 0){
              remove();
          }
          return ret;
    }

    @Override
    public synchronized Object setElementAt(Object obj, int index) {
        return super.setElementAt(obj, index);
    }

    @Override
    public synchronized void reverse(final int index, final int newindex) {
        super.reverse(index, newindex);
    }

    @Override
    public synchronized void removeElementsFrom(final int index, int len) {
        super.removeElementsFrom(index, len);
    }

    @Override
    public synchronized void clear() {
        super.clear();
    }

    @Override
    public synchronized void removeAllElements() {
        super.removeAllElements();
    }

    @Override
    public synchronized Object pop() {
        return super.pop();
    }


    public UniqueArray getDict(){
        Object b = branch;
        while (b instanceof Branch){
             b = ((Branch)b).branch;
        }
        return ((BaseRoot)b).getDict();
    }
    
    public BaseRoot getRoot(){
        Object b = branch;
        while (b instanceof Branch){
            b = ((Branch)b).branch;
        }
        return (BaseRoot)b;

    }

    public Branch newNode(){
        return new Branch(getRoot(), getLength());
    }

    /*public Branch newBranch(){
        return new Branch();
    } */
    public synchronized Branch newNext() {
        return new Branch(getOrNewNode(), getIndex() + 1, getLength());
    }

    /**
     * Mix пучок: grow an isolated leaf chain from this vertex (any node,
     * not only the main root). The new leaf is on the tree; next/prev are
     * not joined to the caller's bunch.
     */
    public Leaf newBunch() {
        return newBunch(getLength());
    }

    public Leaf newBunch(int hopLength) {
        if (this instanceof Leaf)
            return ((Leaf) this).newBunch(hopLength);
        return new Leaf(this, size(), hopLength);
    }
    public synchronized Branch getOrNewNext(){
        Branch branch = (Branch)getOrNewNode().elementAt(getIndex() + 1);
        if (branch == null)
            return  newNext();
        else
            return branch;
    }

    @Override
    public synchronized void insertElementAt(Object obj, int index)
    {
        if (index > amount) {
            index = amount;
        }
        if (amount < arr.length)
            super.insertElementAt(obj, index);
        else {
            Branch branch = getOrNewNext();
            if (branch.amount == branch.arr.length)
                branch = newNext();
            if (index == amount) {
                branch.insertElementAt(obj, 0);
            }else {
                branch.insertElementAt(pop(), 0);
                if (amount == arr.length) {
                    int stop = 1;
                }
                super.insertElementAt(obj, index);
            }
        }
    }

    @Override
    public synchronized int addElement(Object obj)
    {
        insertElementAt(obj, amount);
        return amount;
    }



    @Override
    public synchronized int copyFrom(Object[] from, int offset, int toindex, int len){
        if (len == 0)
            return 0;
        if (offset >= from.length)
            return 0;
        if (offset + len > from.length){
            len = from.length - offset;
        }
        int amount = this.amount;
        Object arr[] = this.arr;
        final int newlen = toindex + len;
        if (newlen > arr.length){
            throw new ArrayIndexOutOfBoundsException();
        }
        if (newlen > amount){
            amount = toindex + len;
            this.amount = amount;
        }
        System.arraycopy(from, offset, arr, toindex, len);
        if (len > 0){
            enumerate(toindex);
        }
        return len;
    }

    @Override
    protected void expand() {
        if (arr == null){
            arr = new Object[getLength()];
        }else {
            throw new IndexOutOfBoundsException("Branch expand");
        }
    }

    @Override
    protected void expandTo(int index){
        if (index >= getLength())
            throw new IndexOutOfBoundsException("Branch expandTo " + index);
        else if (arr == null)
            expand();
    }


    int index = 0;
    Object branch;
    @Override

    public void remove(IVector liveVector){
        if (liveVector == branch){
            branch = null;
            index = 0;
        }
    }

    public void setIndex(IVector v, int val){
        //Object prev = branch;
       /* if ((prev != v) && (prev != null))
            remove();*/
        this.branch = v;
        this.index = val;
    }

    public int getIndex(){
        return index;
    }


    public Branch getPlace(){

        if (branch instanceof Branch)
            return (Branch)branch;
        else
            return null;
    }

    public int getLevel(){
        Branch branch = getPlace();
        int level = 0;
        while (branch !=null){
            level++;
            branch = branch.getPlace();
        }
        return level;
    }



    /**
     * Packed Mix path for KeyDict / cursor order. Mixed radix by each
     * ancestor's {@link #getLength()}; default 32 is the old {@code << 5}.
     * The visible address is {@link #pathString(int)}.
     */
    int getAddress(int fromIndex){
        long address = fromIndex;
        long factor = getLength();
        Branch node = this;
        Branch parent = getPlace();
        while (parent != null){
            address += (long) node.index * factor;
            if (address > Integer.MAX_VALUE)
                throw new IndexOutOfBoundsException("Mix packed address exceeds 32-bit");
            long next = factor * (long) parent.getLength();
            if (factor != 0 && next / factor != parent.getLength())
                throw new IndexOutOfBoundsException("Mix packed address exceeds 32-bit");
            factor = next;
            node = parent;
            parent = node.getPlace();
        }
        return (int) address;
    }

    /**
     * Mix path of ℕ₀ from the root hop to {@code fromIndex} on this page.
     * Example: {@code 12.45.23432.7}.
     */
    public int[] getPath(int fromIndex) {
        int depth = getLevel() + 1;
        int[] path = new int[depth];
        path[depth - 1] = fromIndex;
        Branch node = this;
        int i = depth - 2;
        while (i >= 0 && node != null) {
            path[i] = node.index;
            node = node.getPlace();
            i--;
        }
        return path;
    }

    public String pathString(int fromIndex) {
        int[] path = getPath(fromIndex);
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < path.length; i++) {
            if (i > 0)
                sb.append('.');
            sb.append(path[i]);
        }
        return sb.toString();
    }




}
