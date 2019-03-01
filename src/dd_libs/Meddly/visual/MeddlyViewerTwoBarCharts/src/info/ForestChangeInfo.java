package info;

/**
 * Information Holder Class Containing the necessary info to update a point on
 * the main application histogram
 *
 */
public class ForestChangeInfo {
	/**
	 * The ID of the forest.
	 */
	private int id;

	/**
	 * the Level of the forest to be updated.
	 */
	private int level;

	/**
	 * The active node count change for the update. should be 1 or -1.
	 */
	private int anc;

	/**
	 * Constructor for the ForestChangeInfo holder
	 */
	public ForestChangeInfo(int id, int level, int anc) {
		this.id = id;
		this.level = level;
		this.anc = anc;
	}

	/**
	 * set the level of the change
	 * 
	 * @param level
	 *            the level of the forest at which the change occurs
	 */
	public void addLevel(int level) {
		this.level = level;
	}

	/**
	 * set the active node count change
	 * 
	 * @param anc
	 *            the node change info. must either be either be 1 or -1
	 */
	public void addAnc(int anc) {
		this.anc = anc;
	}

	/**
	 * get the level of the change
	 * 
	 * @return
	 */
	public int getLevel() {
		return this.level;
	}

	/**
	 * get the anc of the change
	 * 
	 * @return
	 * the active node count change
	 */
	public int getAnc() {
		return this.anc;
	}

	/**
	 * sets the forest id of the change
	 * 
	 * @param id
	 * The id of the forest.
	 */
	public void setID(int id) {
		this.id = id;
	}

	/**
	 * gets the forest id of the change
	 * 
	 * @return
	 * The id
	 */
	public int getID() {
		return this.id;
	}

}