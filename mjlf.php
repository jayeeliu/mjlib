<?php
error_reporting(E_ALL ^ E_NOTICE);

$kv = new KVStore('172.16.139.60', 17879);
$table 	= 'tkv';
$key 	= 'key';
$value	= 'breuicnerofneromfoermfoermfoermfoemnroferofoerim';
assert($kv->create($table) === true);
assert($kv->get($table, $key) === '');
assert($kv->put($table, $key, $value) === true);
assert($kv->get($table, $key) === $value);
//var_dump($kv->get($table, $key));
assert($kv->drop($table) === true);


class KVStore {
	private $_fp = null;
	const IS_OK 	= 'OK',
		CMD_GET 	= "get %s %s\r\n",
		CMD_PUT		= "put %s %s %s\r\n",
		CMD_DEL 	= "del %s %s\r\n",
		CMD_CREATE 	= "create %s\r\n",
		CMD_DROP 	= "drop %s\r\n";
	public function __construct($host, $port=17879) {
		$this->_fp = fsockopen($host, $port, $errno, $errmsg, 3);
		if ($errno) {
			throw new Exception($errno, $errno);
		}
	}

	public function get($table, $key) {
		fwrite($this->_fp, sprintf(self::CMD_GET, $table, $key));
		$r_get = $this->_gets();
		if ($this->_is_ok($r_get)) {
			return false;
		}
		return trim(substr($r_get, 2));
	}

	public function put($table, $key, $value) {
		fwrite($this->_fp, sprintf(self::CMD_PUT, $table, $key, $value));
		return self::_is_ok($this->_gets());
	}

	public function del($table, $key) {
		fwrite($this->_fp, sprintf(self::CMD_DEL, $table, $key));
		return self::_is_ok($this->_gets());
	}

	public function create($table) {
		fwrite($this->_fp, sprintf(self::CMD_CREATE, $table));
		return self::_is_ok($this->_gets());
	}

	public function drop($table) {
		fwrite($this->_fp, sprintf(self::CMD_DROP, $table));
		return self::_is_ok($this->_gets());
	}


	private function _gets() {
		if (!$this->_fp) {
			throw new Exception("connection off", 1);
		}
		$ret = '';
		while (!feof($this->_fp)) {
			$ret .= fread($this->_fp, 1024);
		}
		return $ret;
	}

	private function _is_ok($ret) {
		return substr($ret, 0, 2) === self::IS_OK ? false : true;
	}

	public function __destruct() {
		fclose($this->_fp);
	}
}