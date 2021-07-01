package Utils::TextProcessing::PluralToSingular;
use warnings;
use strict;
require Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw/to_singular is_plural/;
our %EXPORT_TAGS = (
    all => \@EXPORT_OK,
);
our $VERSION = '0.22';

# Irregular plurals.

# References:
# http://www.macmillandictionary.com/thesaurus-category/british/Irregular-plurals
# http://web2.uvcs.uvic.ca/elc/studyzone/330/grammar/irrplu.htm
# http://www.scribd.com/doc/3271143/List-of-100-Irregular-Plural-Nouns-in-English

# This mixes latin/greek plurals and anglo-saxon together. It may be
# desirable to split things like corpora and genera from "feet" and
# "geese" at some point.

my %irregular = (qw/
    analyses analysis
    brethren brother
    children child
    corpora corpus
    craftsmen craftsman
    crises crisis
    criteria criterion
    curricula curriculum
    feet foot
    fungi fungus
    geese goose
    genera genus
    gentlemen gentleman
    indices index
    lice louse
    matrices matrix
    memoranda memorandum
    men man
    mice mouse
    monies money
    neuroses neurosis
    nuclei nucleus
    oases oasis
    oxen ox
    pence penny
    people person
    phenomena phenomenon
    quanta quantum
    strata stratum
    teeth tooth
    testes testis
    these this
    theses thesis
    those that
    women woman
ad-men ad-man
admen adman
aircraftmen aircraftman
airmen airman
airwomen airwoman
alaskamen alaskaman
aldermen alderman
anchormen anchorman
ape-men ape-man
assemblymen assemblyman
backwoodsmen backwoodsman
bandsmen bandsman
barmen barman
barrow-men barrow-man
batmen batman
batsmen batsman
beggarmen beggarman
beggarwomen beggarwoman
behmen behman
boatmen boatman
bogeymen bogeyman
bowmen bowman
brakemen brakeman
bushmen bushman
businessmen businessman
businesswomen businesswoman
busmen busman
byre-men byre-man
cabmen cabman
cameramen cameraman
carmen carman
cattlemen cattleman
cavalrymen cavalryman
cavemen caveman
chairmen chairman
chairwomen chairwoman
chapmen chapman
charwomen charwoman
chessmen chessman
chinamen chinaman
churchmen churchman
clansmen clansman
classmen classman
clemen cleman
clergymen clergyman
coachmen coachman
coalmen coalman
cognomen cognoman
con-men con-man
congressmen congressman
congresswomen congresswoman
councilmen councilman
councilwomen councilwoman
countrymen countryman
countrywomen countrywoman
cowmen cowman
cracksmen cracksman
craftsmen craftsman
cragsmen cragsman
crewmen crewman
cyclamen cyclaman
dairymen dairyman
dalesmen dalesman
doormen doorman
draftsmen draftsman
draughtsmen draughtsman
dustmen dustman
dutchmen dutchman
englishmen englishman
englishwomen englishwoman
ex-servicemen ex-serviceman
excisemen exciseman
fellow-men fellow-man
ferrymen ferryman
fieldsmen fieldsman
firemen fireman
fishermen fisherman
flagmen flagman
footmen footman
foremen foreman
forewomen forewoman
freedmen freedman
freemen freeman
frenchmen frenchman
frenchwomen frenchwoman
freshmen freshman
frogmen frogman
frontiersmen frontiersman
g-men g-man
gentlemen gentleman
gentlewomen gentlewoman
germen german
god-men god-man
gombeen-men gombeen-man
groundsmen groundsman
guardsmen guardsman
gunmen gunman
handymen handyman
hangmen hangman
harmen harman
he-men he-man
headmen headman
helmsmen helmsman
hemmen hemman
henchmen henchman
herdsmen herdsman
highwaymen highwayman
horsemen horseman
horsewomen horsewoman
housemen houseman
huntsmen huntsman
husbandmen husbandman
hymen hyman
icemen iceman
indiamen indiaman
infantrymen infantryman
irishmen irishman
irishwomen irishwoman
jazzmen jazzman
journeymen journeyman
jurymen juryman
kinmen kinman
kinsmen kinsman
kinswomen kinswoman
klansmen klansman
landsmen landsman
laundrymen laundryman
laundrywomen laundrywoman
lawmen lawman
laymen layman
liegemen liegeman
liftmen liftman
linemen lineman
linesmen linesman
linkmen linkman
liverymen liveryman
lobstermen lobsterman
longshoremen longshoreman
lumbermen lumberman
madmen madman
madwomen madwoman
mailmen mailman
marksmen marksman
medicine-men medicine-man
men man
merchantmen merchantman
mermen merman
middlemen middleman
midshipmen midshipman
militiamen militiaman
milkmen milkman
minutemen minuteman
motormen motorman
muffin-men muffin-man
musclemen muscleman
needlewomen needlewoman
newsmen newsman
newspapermen newspaperman
newswomen newswoman
night-watchmen night-watchman
noblemen nobleman
nomen noman
norsemen norseman
northmen northman
nurserymen nurseryman
oarsmen oarsman
oarswomen oarswoman
oehmen oehman
oilmen oilman
ombudsmen ombudsman
orangemen orangeman
pantrymen pantryman
patrolmen patrolman
pitchmen pitchman
pitmen pitman
placemen placeman
plainsmen plainsman
ploughmen ploughman
pointsmen pointsman
policemen policeman
policewomen policewoman
postmen postman
potmen potman
pressmen pressman
property-men property-man
quarrymen quarryman
raftsmen raftsman
ragmen ragman
railwaymen railwayman
repairmen repairman
riflemen rifleman
roadmen roadman
roundsmen roundsman
salarymen salaryman
salesmen salesman
saleswomen saleswoman
salmen salman
sandwichmen sandwichman
schoolmen schoolman
scotchmen scotchman
scotchwomen scotchwoman
scotsmen scotsman
scotswomen scotswoman
seamen seaman
seedsmen seedsman
servicemen serviceman
showmen showman
sidesmen sidesman
signalmen signalman
snowmen snowman
specimen speciman
spokesmen spokesman
spokeswomen spokeswoman
sportsmen sportsman
stablemen stableman
stamen staman
stammen stamman
statesmen statesman
steersmen steersman
supermen superman
superwomen superwoman
switchmen switchman
swordsmen swordsman
t-men t-man
tallymen tallyman
taxmen taxman
townsmen townsman
tradesmen tradesman
trainmen trainman
trenchermen trencherman
tribesmen tribesman
turkmen turkman
tutankhamen tutankhaman
underclassmen underclassman
vestrymen vestryman
vonallmen vonallman
washerwomen washerwoman
watchmen watchman
watermen waterman
weathermen weatherman
welshmen welshman
women woman
woodmen woodman
woodsmen woodsman
workmen workman
yachtsmen yachtsman
yeomen yeoman
/);

# Words ending in ves need care, since the ves may become "f" or "fe".

# References:
# http://www.macmillandictionary.com/thesaurus-category/british/Irregular-plurals

my %ves = (qw/
    calves calf
    dwarves dwarf
    elves elf
    halves half
    knives knife
    leaves leaf
    lives life
    loaves loaf
    scarves scarf
    sheaves sheaf
    shelves shelf
    wharves wharf 
    wives wife
    wolves wolf
/);

# A dictionary of plurals.

my %plural = (
    # Words ending in "us" which are plural, in contrast to words like
    # "citrus" or "bogus".
    'menus' => 'menu',
    'buses' => 'bus',
    %ves,
    %irregular,
);

# A store of words which are the same in both singular and plural.

my @no_change = qw/
                      deer
                      ides
                      fish
                      means
                      offspring
                      series
                      sheep
                      species
                  /;

@plural{@no_change} = @no_change;

# A store of words which look like plurals but are not.

# References:

# http://wiki.answers.com/Q/What_are_some_examples_of_singular_nouns_ending_in_S
# http://virtuallinguist.typepad.com/the_virtual_linguist/2009/10/singular-nouns-ending-in-s.html

my @not_plural = (qw/
Aries
Charles
Gonzales 
Hades 
Hercules 
Hermes 
Holmes 
Hughes 
Ives 
Jacques 
James 
Keyes 
Mercedes 
Naples 
Oates 
Raines 
Texas
athletics
bogus
bus
cactus
cannabis
caries
chaos
citrus
clothes
corps
corpus
devious
dias
facies
famous
hippopotamus
homunculus
iris
lens
mathematics
metaphysics
metropolis
mews
minus
miscellaneous
molasses
mrs
narcissus
news
octopus
ourselves
papyrus
perhaps
physics
platypus
plus
previous
pus
rabies
scabies
sometimes
stylus
themselves
this
thus
various
yes
nucleus
synchronous
/);

my %not_plural;

@not_plural{@not_plural} = (1) x @not_plural;

# A store of words which end in "oe" and whose plural ends in "oes".

# References
# http://www.scrabblefinder.com/ends-with/oe/

# Also used

# perl -n -e 'print if /oe$/' < /usr/share/dict/words

my @oes = (qw/
canoes
does
foes
gumshoes
hoes
horseshoes
oboes
shoes
snowshoes
throes
toes
/);

my %oes;

@oes{@oes} = (1) x @oes;

# A store of words which end in "ie" and whose plural ends in "ies".

# References:
# http://www.scrabblefinder.com/ends-with/ie/
# (most of the words are invalid, the above list was manually searched
# for useful words).

# Also get a good list using

# perl -n -e 'print if /ie$/' < /usr/share/dict/words 

# There are too many obscure words there though.

# Also, I'm deliberately not including "Bernie" and "Bessie" since the
# plurals are rare I think. 

my @ies = (qw/
Aussies
Valkryies
aunties
bogies
brownies
calories
charlies
coolies
coteries
curies
cuties
dies
genies
goalies
kilocalories
lies
magpies
menagerie
movies
neckties
pies
porkpies
prairies
quickies
reveries
rookies
sorties
stogies
talkies
ties
zombies
/);

my %ies;

@ies{@ies} = (1) x @ies;

# Words which end in -se, so that we want the singular to change from
# -ses to -se. This also contains verbs like "deceases", so that they
# don't trigger spell checker errors.

my @ses = (qw/
automates
bases
cases
causes
ceases
closes
cornflakes
creases
databases
deceases
flakes
horses
increases
mates
parses
purposes
races
releases
tenses
/);

my %ses;
@ses{@ses} = (1) x @ses;
# A regular expression which matches the end of words like "dishes"
# and "sandwiches". $1 is a capture which contains the part of the
# word which should be kept in a substitution.

my $es_re = qr/([^aeiou]s|ch|sh)es$/;

# Plurals ending -i, singular is either -us, -o or something else
# See https://en.wiktionary.org/wiki/Category:English_irregular_plurals_ending_in_%22-i%22

# -i to -us
my @i_to_us = (qw/
abaci
abaculi
acanthi
acini
alumni
anthocauli
bacilli
baetuli
cacti
calculi
calli
catheti
emboli
emeriti
esophagi
foci
foeti
fumuli
fungi
gonococci
hippopotami
homunculi
incubi
loci
macrofungi
macronuclei
naevi
nuclei
obeli
octopi
oeconomi
oesophagi
panni
periœci
phocomeli
phoeti
platypi
polypi
precunei
radii
rhombi
sarcophagi
solidi
stimuli
succubi
syllabi
thesauri
thrombi
tori
trophi
uteri
viri
virii
xiphopagi
zygomatici
/);

my %i_to_us;
@i_to_us{@i_to_us} = (1) x @i_to_us;

# -i to -o
my @i_to_o = (qw/
    alveoli
    ghetti
    manifesti
    ostinati
    pianissimi
    scenarii
    stiletti
    torsi
/);

my %i_to_o;
@i_to_o{@i_to_o} = (1) x @i_to_o;

# -i to something else

my %i_to_other = (
    improvisatori => 'improvisatore',
    rhinoceri => 'rhinoceros',
    scaloppini => 'scaloppine'
);

# See documentation below.

sub to_singular
{
    my ($word) = @_;
    # The return value.
    my $singular = $word;
    if (! $not_plural{$word}) {
        # The word is not in the list of exceptions.
        if ($plural{$word}) {
            # The word has an irregular plural, like "children", or
            # "geese", so look up the singular in the table.
            $singular = $plural{$word};
        }
        elsif ($word =~ /s$/) {
            # The word ends in "s".
            if ($word =~ /'s$/) {
            # report's, etc.
            ;
            }
            elsif (length ($word) <= 2) {
            # is, as, letter s, etc.
            ;
            }
            elsif ($word =~ /ss$/) {
            # useless, etc.
            ;
            }
            elsif ($word =~ /sis$/) {
            # basis, dialysis etc.
            ;
            }
            elsif ($word =~ /ies$/) {
                # The word ends in "ies".
                if ($ies{$word}) {
                    # Lies -> lie
                    $singular =~ s/ies$/ie/;
                }
                else {
                    # Fries -> fry
                    $singular =~ s/ies$/y/;
                }
            }
            elsif ($word =~ /oes$/) {
                # The word ends in "oes".
                if ($oes{$word}) {
                    # Toes -> toe
                    $singular =~ s/oes$/oe/;
                }
                else {
                    # Potatoes -> potato
                    $singular =~ s/oes$/o/;
                }
            }
            elsif ($word =~ /xes$/) {
                # The word ends in "xes".
		        $singular =~ s/xes$/x/;
            }
            elsif ($word =~ /ses$/) {
                if ($ses{$word}) {
                    $singular =~ s/ses$/se/;
                }
                else {
                    $singular =~ s/ses$/s/;
                }
	        }
            elsif ($word =~ $es_re) {
                # Sandwiches -> sandwich
                # Dishes -> dish
                $singular =~ s/$es_re/$1/;
            }
            else {
                # Now the program has checked for every exception it
                # can think of, so it assumes that it is OK to remove
                # the "s" from the end of the word.
                $singular =~ s/s$//;
            }
        }
        elsif ($word =~ /i$/) {
            if ($i_to_us{$word}) {
                $singular =~ s/i$/us/;
            }
            elsif ($i_to_o{$word}) {
                $singular =~ s/i$/o/;
            }
            if ($i_to_other{$word}) {
                $singular = $i_to_other{$word};
            }
        }

    }
    return $singular;
}

sub is_plural
{
    my ($word) = @_;
    my $singular = to_singular ($word);
    my $is_plural;
    if ($singular ne $word) {
	$is_plural = 1;
    }
    elsif ($plural{$singular} && $plural{$singular} eq $singular) {
	$is_plural = 1;
    }
    else {
	$is_plural = 0;
    }
    return $is_plural;
}

1;
