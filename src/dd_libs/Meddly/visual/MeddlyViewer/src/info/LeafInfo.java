
// $Id$

/*
    Meddly: Multi-terminal and Edge-valued Decision Diagram LibrarY.
    Copyright (C) 2009, Iowa State University Research Foundation, Inc.

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published 
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

package info;

import java.util.PriorityQueue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * 
 * @author Coleman
 *
 */
public class LeafInfo {
	private LinkedBlockingQueue<Integer> level;
	private LinkedBlockingQueue<Integer> anc;

	/**
	 * 
	 */
	public LeafInfo() {
		this.level = new LinkedBlockingQueue<Integer>();
		this.anc = new LinkedBlockingQueue<Integer>();
	}

	/**
	 * 
	 * @param level
	 */
	public void addLevel(int level) {
		this.level.add(level);
	}

	/**
	 * 
	 * @param anc
	 */
	public void addAnc(int anc) {
		this.anc.add(anc);
	}

	/**
	 * 
	 * @return
	 */
	public int getLevel() {
		return this.level.poll();
	}

	/**
	 * 
	 * @return
	 */
	public int getAnc() {
		return this.anc.poll();
	}

	/**
	 * 
	 * @return
	 */
	public boolean hasNext() {
		if (level.peek() == null || anc.peek() == null)
			return false;

		return true;
	}

}
