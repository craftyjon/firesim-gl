class Fixture
{
  public:
    Fixture()
};

class Strand
{
  public:
    Strand(int id) : _id(id) {}

    void addFixture(unique_ptr<Fixture> fixture) {
        assert(!finalized);
        fixtures.push_back(move(fixture));
    }
    void finalize();

  private:
    bool finalized = false;
    int id;
    vector<unique_ptr<Fixture> > fixtures;
};

class Scene
{
  public:
    Scene(const char *name);

    QPoint getCenter() const { return _center; }
    QPoint getCorner() const { return _corner; }

    bool haveStrand(int id) { return id < _strands.size() && _strands[id]; }
    Strand *getStrand(int id) { return _strands[id]; }
    const Strand *getStrand(int id) const { return _strands[id]; }

  private:
    QPoint _center;
    QPoint _corner;
    vector<unique_ptr<Strand> > _strands;
};
