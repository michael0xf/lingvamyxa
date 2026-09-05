
package com.mtk.map;

/**
 * @author <a href="mailto:mtkravchenko@gmail.com">Mikhail Kravchenko</a>
 *
 * Porting (Lingvamyxa MIX / L2): Java monitors on the overlapping tree.
 * MapThreads used to serialize edits, so these locks were dropped. Restore
 * them so two writers on one live document are safe to port.
 *
 * Protocol:
 * 1. Every node is 32 slots ({@link #SIZE} = 5 bits). Same page for Leaf
 *    and internal Branch: overflow memmove stays short, address digits match.
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



    final static int SIZE = 5;
    final static int LENGTH = 1 << SIZE;

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
        branch = tipTop;
        array(LENGTH);
    }

    Branch(Branch node, int at){
        array(LENGTH);
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
        return LENGTH;
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
        return new Branch(getRoot());
    }

    /*public Branch newBranch(){
        return new Branch();
    } */
    public synchronized Branch newNext() {
        return new Branch(getOrNewNode(), getIndex() + 1);
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
            arr = new Object[LENGTH];
        }else {
            throw new IndexOutOfBoundsException("Branch expand");
        }
    }

    @Override
    protected void expandTo(int index){
        if (index >= LENGTH)
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



    int getAddress(int fromIndex){
        int address = fromIndex;
        int level = 1;
        Branch branch = getPlace();
        int index = this.index;
        while (branch != null){
            address += (index << (level * Branch.SIZE));
            /*if (this instanceof Leaf){
                if (((Leaf)this).prev != null){
                    int stop = 1;
                }

            } */
            level++;
            index = branch.index;
            branch = branch.getPlace();
        }
        return address;
    }




}
